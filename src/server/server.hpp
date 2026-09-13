#pragma once

#include <memory>
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

/*
 * @brief Server class that handles incoming requests and manages the robot.
 *
 * Always call `wait()` after `stop()` to ensure the server thread has finished.
 */
class Server
{
public:
    Server(const std::string& ip, int port, const std::string& conf_filepath);
    ~Server();

    void start();
    void stop();

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

    std::unique_ptr<robot::Robot> robot_;

    void loop_();
    void robot_worker_loop_();
    void sweeper_loop_();
};

} // namespace robert
