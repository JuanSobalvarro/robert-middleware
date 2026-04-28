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
    RapidRequest command;
    std::shared_ptr<std::promise<std::vector<uint8_t>>> response_promise;
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
    std::future<std::vector<uint8_t>> queue_request(const RapidRequest& request);

private:
    void worker_loop();

    bool send_request(const RapidRequest& request);
    std::vector<uint8_t> receive_buffer();
    std::vector<uint8_t> send_and_receive(const RapidRequest& request);

    bool attempt_connection();

    std::string ip_;
    int port_;

    // socket owned by Robot
    int socket_fd_{-1};
    int socket_timeout_ms_{35000}; // 35 seconds timeout for send/recv

    // worker thread
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::mutex socket_mutex_;

    // request queue
    std::queue<RobotWorkItem> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // connection state
    std::atomic<bool> connected_{false};
};

} // namespace robert