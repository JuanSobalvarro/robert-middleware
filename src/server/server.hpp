#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <zmq.hpp>

#include "commands/commands.hpp"
#include "protocol/protocol.pb.h"
#include "robot/robot.hpp"
#include "server/tasker.hpp"
#include "tasker.hpp"

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

private:
    Tasker tasker_;
    std::string ip_;
    int port_;
    std::thread server_thread_;
    std::thread robot_worker_thread_;
    std::atomic<bool> running_{false};

    zmq::context_t context_;
    zmq::socket_t socket_;

    std::vector<std::unique_ptr<robot::Robot>> robots_;

    protocol::ServerResponse process_request(const commands::DecodedRequest& decoded);

    void loop_();
    void robot_worker_loop_();
};

} // namespace robert
