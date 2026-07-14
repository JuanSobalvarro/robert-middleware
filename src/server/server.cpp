#include "server/server.hpp"
#include "commands/commands.hpp"
#include "commands/decoder.hpp"
#include "protocol/protocol.pb.h"

#include <future>
#include <iostream>
#include <fstream>


namespace robert::server {

Server::Server(const std::string& ip, int port)
    : ip_(ip), port_(port), context_(1), socket_(context_, ZMQ_REP)
{
    std::string address = "tcp://" + ip_ + ":" + std::to_string(port_);
    socket_.bind(address);
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    if (running_) return;

    // check for robots
    if (robots_.empty())
    {
        std::cout << "[MIDDLEWARE] Maybe you dont want to run the server without robots added ? u.u" << std::endl;
    }

    std::cout << "[MIDDLEWARE] Starting Robots' sessions..." << std::endl;

    for (auto& robot : robots_)
    {
        robot->start_session();
    }
    std::cout << "[MIDDLEWARE] Robots' sessions ready!" << std::endl;

    running_ = true;
    server_thread_ = std::thread(&Server::loop_, this);
    robot_worker_thread_ = std::thread(&Server::robot_worker_loop_, this);
}

void Server::stop()
{
    std::cout << "[SERVER] Stopping server" << std::endl;

    running_ = false;

    tasker_.stop();

    if (server_thread_.joinable())
    {
        server_thread_.join();
    }

    if (robot_worker_thread_.joinable())
    {
        robot_worker_thread_.join();
    }

    for (auto& robot : robots_)
    {
        robot->stop_session();
    }

    std::cout << "[MIDDLEWARE] Server stopped and all robot sessions closed." << std::endl;
}

void Server::load_robots_from_file(const std::string& filepath)
{
    std::ifstream file(filepath);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "[ERROR] Couldnt open robots config file at: " << filepath << std::endl;
    }

    while(std::getline(file, line))
    {
        // skip if empty or the first line with the headers
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string name, password, ip, port_str;

        if (std::getline(ss, name, '|') &&
            std::getline(ss, password, '|') &&
            std::getline(ss, ip, '|') &&
            std::getline(ss, port_str, '|'))
        {
            int port = std::stoi(port_str);
            std::cout << "[Config] Adding robot: " << name << " at " << ip << ":" << port_str << std::endl;

            robots_.push_back(std::make_unique<robot::Robot>(ip, port));
        }
    }
}

void Server::wait() {
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    if (robot_worker_thread_.joinable()) {
        robot_worker_thread_.join();
    }
}

void Server::loop_()
{
    std::cout << "[MIDDLEWARE] ZeroMQ Server listening on " << ip_ << ":" << port_ << std::endl;
    while(running_)
    {
        zmq::message_t request;
        std::cout << "[SERVER] Loop waiting for recv" << std::endl;
        // remember that recv is thread blocking
        zmq::recv_result_t res = socket_.recv(request, zmq::recv_flags::none);

        if (res.has_value() && (EAGAIN == res.value())) {
            throw std::runtime_error("[ERROR] There is an error with the recv");
        }

        std::string buffer(static_cast<char*>(request.data()), request.size());
        protocol::ServerResponse pb_response;

        try {
            const commands::DecodedRequest decoded = commands::Decoder::decode_buffer(buffer);

            pb_response = process_request(decoded);
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
            socket_.send(reply, zmq::send_flags::none);

            continue;
        }

        zmq::message_t reply(binary_payload.size());
        memcpy(reply.data(), binary_payload.data(), binary_payload.size());
        socket_.send(reply, zmq::send_flags::none);
    }
    std::cout << "[SERVER] Ended main loop" << std::endl;
}

void Server::robot_worker_loop_() {
    while (running_) {
        // wait for a task to be added from the server thread
        std::cout << "[SERVER_WORKER] Waiting for next task" << std::endl;
        auto next_id = tasker_.waitForNextTask();

        if (!next_id) {
            std::cout << "[SERVER_WORKER] Received null next id, shutting down robot worker" << std::endl;
            break;
        }

        task_id_t task_id = *next_id;

        std::cout << "[SERVER_WORKER] Task with id " << task_id << " in operation" << std::endl;

        // once we got the task then we start it
        if (!tasker_.startTask(task_id)) continue;

        // safely get the payload
        auto task = tasker_.getTask(task_id);

        if (!task) continue;

        const commands::RapidRequest& req = task->getRequest();

        // send the request to the robot and BLOCK only THIS thread
        if (!robots_.empty() && robots_[0]->is_connected()) {
            std::future<std::vector<uint8_t>> future_ack = robots_[0]->queue_request(req);

            if (future_ack.wait_for(std::chrono::seconds(90)) == std::future_status::ready) {
                std::vector<uint8_t> raw_response = future_ack.get();
                std::string response(raw_response.begin(), raw_response.end());

                tasker_.completeTask(task_id, response);
            } else {
                tasker_.failTask(task_id); // Timeout
            }
        } else {
            tasker_.failTask(task_id); // Robot disconnected
        }
    }
    std::cout << "[SERVER_WORKER] Ended loop worker" << std::endl;
}

protocol::ServerResponse Server::process_request(const commands::DecodedRequest& decoded)
{
    protocol::ServerResponse response;

    std::cout << "[SERVER] Processing Request" << std::endl;

    switch (decoded.cmd_type)
    {
    case commands::RapidCommandType::UNKNOWN:
        response.set_status(protocol::ResponseStatus::ERROR);
        response.set_error_message("Unknown command");
        break;

    case commands::RapidCommandType::EXIT:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_text_payload("OKISUWU");
        std::cout << "exit called, running_ to false" << std::endl;
        running_ = false;
        break;

    case commands::RapidCommandType::PING:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_text_payload("PONGUWU");
        break;

    case commands::RapidCommandType::PINGR:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_error_message("PONGR_NOT_IMPLEMENTED");
        break;

    case commands::RapidCommandType::CHECK_TASK:
    {
        task_id_t id = decoded.task_id.value();
        auto task = tasker_.getTask(id);

        if (!task) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_error_message("Task ID not found in registry");
            break;
        }

        TaskState state = task->getState();
        const std::string& message = task->getResponse();

        if (state == TaskState::COMPLETED) {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_task_status(protocol::TaskStatus::TASK_COMPLETED);

            if (task->getRequest().command_id == commands::RapidCommandType::GETSTATUS) {
                const std::vector<uint8_t> raw_data(message.begin(), message.end());
                bool success = commands::Decoder::unpack_robot_status(raw_data, response.mutable_robot_status());

                if (!success) {
                    response.set_status(protocol::ResponseStatus::ERROR);
                    response.set_error_message("Failed to unpack telemetry payload");
                }
            } else {
                response.set_text_payload(message);
            }
            tasker_.removeTask(id);
        }
        else if (state == TaskState::PENDING) {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_task_status(protocol::TaskStatus::TASK_PENDING);
        }
        else if (state == TaskState::IN_PROGRESS) {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_task_status(protocol::TaskStatus::TASK_IN_PROGRESS);
        }
        else {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_task_status(protocol::TaskStatus::TASK_FAILED);
            response.set_error_message("Task execution failed at robot controller");
        }
        break;
    }
    // command for robot, so queue it and return success
    default:
        if (robots_.empty() || !robots_[0]->is_connected()) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_error_message("Robot is disconnected");
            break;
        }

        const commands::RapidRequest robot_req = create_rapid_request(decoded);
        task_id_t new_task_id = tasker_.addTask(robot_req);

        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_task_status(protocol::TaskStatus::TASK_PENDING); // Inicializado como PENDING en lugar de IN_PROGRESS
        response.set_task_id(new_task_id);
        response.set_text_payload("uwunyanichan");
        break;
    }

    std::cout << "[SERVER] Correctly returning response" << std::endl;

    return response;
}

} // namespace robert::server
