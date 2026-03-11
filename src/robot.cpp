#include "robot.hpp"


robert::Robot::Robot(const std::string& ip, int port)
    : ip_(ip), port_(port)
{
}

robert::Robot::~Robot()
{
    stop_session();
}

void robert::Robot::start_session()
{
    
    std::cout << "[ROBOT] Starting thread for Robot at " << ip_ << ":" << port_ << std::endl;
    running_ = true;
    worker_thread_ = std::thread(&Robot::worker_loop, this);
}

std::string robert::Robot::send_and_receive(const std::string& message)
{
    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ == -1)
    {
        throw std::runtime_error("Robot session is not active");
    }

    if (!send_string(message))
    {
        throw std::runtime_error("Failed to send command to robot");
    }

    return receive_string();
}

bool robert::Robot::send_string(const std::string& message)
{
    std::size_t total_sent = 0;
    const char* data = message.c_str();
    const std::size_t size = message.size();

    while (total_sent < size)
    {
        const ssize_t bytes_sent = send(socket_fd_, data + total_sent, size - total_sent, 0);
        if (bytes_sent <= 0)
        {
            return false;
        }

        total_sent += static_cast<std::size_t>(bytes_sent);
    }

    return true;
}

std::string robert::Robot::receive_string()
{
    char buffer[256]{};
    const ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0)
    {
        throw std::runtime_error("Failed to receive response from robot");
    }

    return std::string(buffer, static_cast<std::size_t>(bytes_received));
}

void robert::Robot::stop_session()
{
    running_ = false;
    queue_cv_.notify_all();

    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }

    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (socket_fd_ != -1)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

void robert::Robot::queue_message(const std::string& message)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        message_queue_.push(message);
    }
    queue_cv_.notify_one();
}

bool robert::Robot::attempt_connection()
{
    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ != -1) return true;

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) return false;

    struct timeval timeout;
    timeout.tv_sec = 2; // 2 seconds
    timeout.tv_usec = 0;
    
    setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &server_addr.sin_addr);

    std::cout << "[ROBOT] Attempting connection to " << ip_ << ":" << port_ << "..." << std::endl;

    if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }   

    connected_ = true;
    return true;
}

void robert::Robot::worker_loop()
{
    std::cout << "[ROBOT::WORKER] Hi from worker thread for Robot at " << ip_ << ":" << port_ << std::endl;
    while (running_)
    {
        if (!connected_)
        {
            if (attempt_connection())
            {
                std::cout << "[ROBOT::WORKER] Successfully connected to " << ip_ << ":" << port_ << std::endl;
            }
            else
            {
                std::cout << "[ROBOT::WORKER] Failed to connect to " << ip_ << ":" << port_ << ", retrying in 5 seconds..." << std::endl;
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait_for(lock, std::chrono::seconds(5), [this]{ return !running_; });
                continue;
            }
        }

        std::string message;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !message_queue_.empty() || !running_; });

            if (!running_ && message_queue_.empty())
                break;

            message = message_queue_.front();
            message_queue_.pop();
        }

        try
        {
            std::cout << "[ROBOT::WORKER] Sending command: " << message << std::endl;
            const std::string response = send_and_receive(message);
            if (response.empty())
            {
                std::cerr << "[ROBOT::WORKER] Empty response received" << std::endl;
                continue;
            }
            std::cout << "[ROBOT::WORKER] Response: " << response << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[ROBOT::WORKER] ERROR: " << e.what() << std::endl;
        }
    }
    std::cout << "[ROBOT::WORKER] Worker thread exiting for Robot at " << ip_ << ":" << port_ << std::endl;
}