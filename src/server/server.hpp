#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <zmq.hpp>

#include "robot/robot.hpp"
#include "server/tasker.hpp"
#include "tasker.hpp"
#include "session.hpp"
#include "request_handler.hpp"

namespace robert::server
{

class Server
{
public:
    Server(const std::string& ip, int port);
    ~Server();

    void start();
    void stop();

    void load_robots_from_file(const std::string& filepath);
    void load_users_from_file(const std::string& filepath);

    void wait();

private:
    Tasker tasker_;
    SessionManager session_manager_;
    RequestHandler request_handler_;
    std::string ip_;
    int port_;
    std::thread server_thread_;
    std::thread robot_worker_thread_;
    std::thread sweeper_thread_;
    std::atomic<bool> running_{false};

    zmq::context_t context_;
    zmq::socket_t socket_server_;

    std::vector<std::unique_ptr<robot::Robot>> robots_;

    void loop_();
    void robot_worker_loop_();
    void sweeper_loop_();
};

} // namespace robert
