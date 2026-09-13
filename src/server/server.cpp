#include "server/server.hpp"
#include "commands/commands.hpp"
#include "commands/decoder.hpp"
#include "protocol/protocol.pb.h"
#include "robot/robot.hpp"
#include "server/session.hpp"
#include "zmq.hpp"
#include "core/conf_parser.hpp"

// undefine ERROR macro to avoid conflicts with Windows API
#ifdef _WIN32
    #undef ERROR
#endif

namespace robert::server {

Server::Server(const std::string& ip, int port, const std::string& conf_filepath)
    : ip_(ip), port_(port), context_(1), socket_server_(context_, zmq::socket_type::rep), request_handler_(session_manager_, tasker_, robot_)
{
    socket_server_.set(zmq::sockopt::rcvtimeo, 500);
    std::string address = "tcp://" + ip_ + ":" + std::to_string(port_);
    socket_server_.bind(address);

    parser::Conf conf = parser::parse_conf(conf_filepath);

    // load robot
    robot_ = std::make_unique<robot::Robot>(
        conf.robot.ip,
        conf.robot.port,
        conf.robot.timeout
    );

    // load users
    session_manager_.load_users(conf.users);
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    if (running_) return;

    // check for robots
    if (robot_ == nullptr) {
        std::cerr << "[SERVER] No robot loaded. Please load a robot before starting the server." << std::endl;
        return;
    }

    if (session_manager_.num_users() == 0) {
        std::cerr << "[SERVER] No users loaded. Please load users from a file before starting the server." << std::endl;
        return;
    }

    std::cout << "[SERVER] Starting robot session" << std::endl;

    robot_->start_session();
    std::cout << "[SERVER] Robot session (worker thread) ready." << std::endl;

    running_ = true;
    server_thread_ = std::thread(&Server::loop_, this);
    robot_worker_thread_ = std::thread(&Server::robot_worker_loop_, this);
    sweeper_thread_ = std::thread(&Server::sweeper_loop_, this);
}

void Server::stop()
{
    // if server is not running, do nothing
    if (!running_) return;

    std::cout << "[SERVER] Stopping server" << std::endl;

    running_ = false;

    tasker_.stop();

    robot_->stop_session();

    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    if (robot_worker_thread_.joinable()) {
        robot_worker_thread_.join();
    }

    if (sweeper_thread_.joinable()) {
        sweeper_thread_.join();
    }

    std::cout << "[SERVER] Server stopped and all robot sessions closed." << std::endl;
}

void Server::loop_()
{
    std::cout << "[MIDDLEWARE] ZeroMQ Server listening on " << ip_ << ":" << port_ << std::endl;
    while(running_)
    {
        zmq::message_t request;
        // std::cout << "[SERVER] Loop waiting for recv" << std::endl;
        // remember that recv is thread blocking
        zmq::recv_result_t res = socket_server_.recv(request, zmq::recv_flags::none);

        // if there is no value, the recv timed out or there was an error
        if (!res.has_value()) {
            // std::cout << "[SERVER] No value received, continuing loop" << std::endl;
            continue;
        }

        if (EAGAIN == res.value()) {
            throw std::runtime_error("[ERROR] There is an error with the recv");
        }

        std::string buffer(static_cast<char*>(request.data()), request.size());
        protocol::ServerResponse pb_response;

        try {
            const commands::DecodedRequest decoded = commands::Decoder::decode_buffer(buffer);

            pb_response = request_handler_.handle(decoded, running_);
        }
        catch (const std::exception& e) {
            std::cerr << "[SERVER ERROR] " << e.what() << std::endl;
            pb_response.set_status(protocol::ResponseStatus::ERROR);
            pb_response.set_error_message(std::string("ERR_SERVER_EXCEPTION: ") + e.what());
        }

        std::string binary_payload;
        bool success = pb_response.SerializeToString(&binary_payload);
        if (!success) {
            std::cerr << "[ERROR] Failed to serialize response" << std::endl;

            std::string fallback = "ERR_SERIALIZATION_FAILED";
            zmq::message_t reply(fallback.size());
            memcpy(reply.data(), fallback.data(), fallback.size());
            socket_server_.send(reply, zmq::send_flags::none);

            continue;
        }

        zmq::message_t reply(binary_payload.size());
        memcpy(reply.data(), binary_payload.data(), binary_payload.size());
        socket_server_.send(reply, zmq::send_flags::none);
    }
    std::cout << "[SERVER] Ended main loop" << std::endl;
}

void Server::robot_worker_loop_() {
    while (running_) {
        // wait for a task to be added from the server thread
        std::cout << "[SERVER_ROBOT_WORKER] Waiting for next task" << std::endl;
        auto next_id = tasker_.waitForNextTask();

        if (!next_id) {
            std::cout << "[SERVER_ROBOT_WORKER] Received null next id, shutting down robot worker" << std::endl;
            break;
        }

        task_id_t task_id = *next_id;

        std::cout << "[SERVER_ROBOT_WORKER] Task with id " << task_id << " in operation" << std::endl;

        // once we got the task then we start it
        if (!tasker_.startTask(task_id))
            continue;

        // safely get the payload
        auto task = tasker_.getTask(task_id);

        if (!task)
            continue;

        const commands::RapidRequest& req = task->getRequest();

        // send request and block only this thread
        if (!robot_->is_connected()) {
            tasker_.failTask(task_id, "Robot disconnected");
            continue;
        }
        std::future<std::vector<uint8_t>> future_ack = robot_->queue_request(req);

        if (future_ack.wait_for(std::chrono::seconds(90)) == std::future_status::ready) {
            std::vector<uint8_t> raw_response = future_ack.get();
            std::string response(raw_response.begin(), raw_response.end());

            if (response.find("ERR:") == 0 || response.find("NACK") == 0) {
                tasker_.failTask(task_id, "Robot error: " + response);
            } else {
                tasker_.completeTask(task_id, response);
            }
        } else {
            tasker_.failTask(task_id, "Robot disconnected"); // Robot disconnected
        }
    }
    std::cout << "[SERVER_WORKER] Ended loop worker" << std::endl;
}

void Server::sweeper_loop_() {
    while (running_) {
        std::cout << "[SWEEPER] Sweeping expired sessions and tasks" << std::endl;

        for (int i = 0; i < 5 && running_; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (running_) {
            session_manager_.sweep_expired_sessions(std::chrono::seconds(60));
            tasker_.sweepExpiredTasks(std::chrono::seconds(300));
        }

    }
}

} // namespace robert::server
