#pragma once

#include <string>
#include <iostream>
#include <atomic>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <thread>
#include <future>
#include <zmq.hpp>

#include "decoder.hpp"
#include "command.hpp"
#include "robot.hpp"
#include "utils.hpp"
#include "protocol/protocol.pb.h" 

namespace robert
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
    std::string ip_;
    int port_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};

    zmq::context_t context_;
    zmq::socket_t socket_;

    std::vector<std::unique_ptr<Robot>> robots_;

    protocol::ServerResponse process_request(const DecodedRequest& decoded);

    void loop_();
};

} // namespace robert