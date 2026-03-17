#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <future>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "command.hpp"

namespace robert {

struct RobotWorkItem {
    MessageCommand command;
    std::shared_ptr<std::promise<std::string>> response_promise;
};

class Robot {
public:
    Robot(const std::string& ip, int port, int timeout_ms = 5000);
    ~Robot() noexcept;

    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;

    void start_session();
    void stop_session();

    bool is_connected() const;

    // Thread-safe
    std::future<std::string> queue_message(const MessageCommand& message);

private:
    void worker_loop();

    bool send_message(const MessageCommand& message);
    std::string send_and_receive(const MessageCommand& message);
    std::string receive_string();

    bool attempt_connection();

    std::string ip_;
    int port_;

    // socket owned by Robot
    int socket_fd_{-1};
    int socket_timeout_ms_{5000}; // 5 seconds timeout for send/recv

    // worker thread
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::mutex socket_mutex_;

    // message queue
    std::queue<RobotWorkItem> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // connection state
    std::atomic<bool> connected_{false};
};

} // namespace robert