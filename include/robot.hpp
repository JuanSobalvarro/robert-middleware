#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <queue>
#include <iostream>
#include <stdexcept>
#include <atomic>

namespace robert {

class Robot {
public:
    Robot(const std::string& ip, int port);
    ~Robot();

    void start_session();
    void stop_session();

    void queue_message(const std::string& message);
    
private:
    void worker_loop(); // thread function to handle sending/receiving messages

    std::string send_and_receive(const std::string& message);
    bool send_string(const std::string& message);
    std::string receive_string();

    std::string ip_;
    int port_;
    int socket_fd_{-1};
    
    // threading
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::mutex socket_mutex_;

    // queue
    std::queue<std::string> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // connection
    std::atomic<bool> connected_{false};
    bool attempt_connection();
};

} // namespace robert