#include "robot.hpp"
#include "commands/commands.hpp"
#include "debug.hpp"

#include <iostream>
#include <string>

namespace robert::robot {
Robot::Robot(const std::string& host, int port, int timeout_ms)
    : host_(host), port_(port), socket_timeout_ms_(timeout_ms)
{
}

Robot::~Robot() noexcept {
    stop_session();
}

void Robot::start_session() {
    if (running_)
        return;

    DEBUG_LOG("[ROBOT] Starting thread for Robot at "
              + host_ + ":" + std::to_string(port_));

    running_ = true;
    worker_thread_ = std::thread(&Robot::worker_loop, this);
}

void Robot::stop_session() {
    running_ = false;
    queue_cv_.notify_all();

    if (worker_thread_.joinable())
        worker_thread_.join();

    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ != INVALID_SOCKET_FD) {
        sock_comm::close_socket(socket_fd_);
        socket_fd_ = INVALID_SOCKET_FD;
    }

    connected_ = false;
}

bool Robot::is_connected() const {
    return connected_;
}

std::future<std::vector<uint8_t>> Robot::queue_request(const commands::RapidRequest& request) {
    if (!running_)
        throw std::runtime_error("Robot session not started");

    auto promise = std::make_shared<std::promise<std::vector<uint8_t>>>();
    std::future<std::vector<uint8_t>> future = promise->get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push({request, promise});
    }

    queue_cv_.notify_one();
    return future;
}

std::vector<uint8_t> Robot::send_and_receive(const commands::RapidRequest& request) {
    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ == INVALID_SOCKET_FD)
        throw std::runtime_error("Robot session not active");

    if (!send_request(request))
        throw std::runtime_error("Failed to send command to robot");

    return receive_buffer();
}

bool Robot::send_request(const commands::RapidRequest& request) {
    std::size_t total_sent = 0;
    const char* data_ptr = reinterpret_cast<const char*>(&request);

    while (total_sent < sizeof(request)) {
        ssize_t bytes_sent = send(
            socket_fd_,
            data_ptr + total_sent,
            static_cast<int>(sizeof(request) - total_sent),
            0
        );

        if (bytes_sent <= 0)
            return false;

        total_sent += static_cast<std::size_t>(bytes_sent);
    }

    std::cout << "[ROBOT] Sent command ID: "
              << static_cast<int>(request.command_id)
              << " (total " << total_sent << " bytes)"
              << std::endl;

    return true;
}

std::vector<uint8_t> Robot::receive_buffer() {
    char buffer[256]{};

    sock_comm::set_timeouts(socket_fd_, socket_timeout_ms_);

    const ssize_t bytes_received = recv(
        socket_fd_,
        buffer,
        static_cast<int>(sizeof(buffer) - 1),
        0
    );

    if (bytes_received <= 0)
        throw std::runtime_error("Failed to receive response from robot");

    return std::vector<uint8_t>(buffer, buffer + static_cast<std::size_t>(bytes_received));
}

bool Robot::attempt_connection() {
    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ != INVALID_SOCKET_FD)
        return true;

    std::cout << "[ROBOT] Attempting connection to "
                  << host_ << ":" << port_ << "..." << std::endl;

    socket_fd_ = sock_comm::connect_tcp(host_, port_, socket_timeout_ms_);

    if (socket_fd_ == INVALID_SOCKET_FD)
        return false;

    connected_ = true;

    return true;
}

void Robot::worker_loop() {
    DEBUG_LOG("[ROBOT::WORKER] Worker thread started for Robot at "
              + host_ + ":" + std::to_string(port_));

    while (running_) {
        if (!connected_) {
            if (attempt_connection()) {
                std::cout << "[ROBOT::WORKER] Connected to " + host_ + ":" + std::to_string(port_) << std::endl;
            }
            else {
                std::cout << "[ROBOT::WORKER] Connection failed, retrying in 5 seconds..."
                          << std::endl;

                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait_for(
                    lock,
                    std::chrono::seconds(5),
                    [this] { return !running_; }
                );

                continue;
            }
        }

        RobotWorkItem work_item;
        bool is_heartbeat = false;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            bool has_work = queue_cv_.wait_for(
                lock,
                std::chrono::seconds(2),
                [this] { return !request_queue_.empty() || !running_; }
            );

            if (!running_)
                break;

            if (has_work) {
                work_item = request_queue_.front();
                request_queue_.pop();
            } else {
                is_heartbeat = true;
                work_item.command.command_id = commands::RapidCommandType::PINGR;
                DEBUG_LOG("[ROBOT::WORKER] No work to do, waiting for heartbeat");
            }
        }

        try {
            const std::vector<uint8_t> response = send_and_receive(work_item.command);

            // if it's a heartbeat, skip the response handling (promise)
            if (is_heartbeat) {
                continue;
            }

            if (response.empty()) {
                std::cerr << "[ROBOT::WORKER] Empty response received"
                          << std::endl;

                std::string error_msg = "ERR: empty response";
                work_item.response_promise->set_value(std::vector<uint8_t>{error_msg.begin(), error_msg.end()});
                continue;
            }

            work_item.response_promise->set_value(response);
        } catch (const std::exception& e) {
            std::cerr << "[ROBOT::WORKER] ERROR: "
                      << e.what() << std::endl;

            {
                std::lock_guard<std::mutex> lock(socket_mutex_);

                if (socket_fd_ != INVALID_SOCKET_FD) {
                    sock_comm::close_socket(socket_fd_);
                    socket_fd_ = INVALID_SOCKET_FD;
                }

                connected_ = false;
            }

            if (!is_heartbeat) {
                std::string error_msg = "ERR: " + std::string(e.what());
                work_item.response_promise->set_value(
                    std::vector<uint8_t>{error_msg.begin(), error_msg.end()}
                );
            }
        }
    }

    DEBUG_LOG("[ROBOT::WORKER] Worker thread exiting for Robot at "
              + host_ + ":" + std::to_string(port_));
}

} // namespace robert::robot
