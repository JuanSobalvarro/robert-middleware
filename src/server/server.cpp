#include "server/server.hpp"
#include "commands/commands.hpp"
#include "commands/decoder.hpp"

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

    std::cout << "[MIDDLEWARE] ZeroMQ Server listening on " << ip_ << ":" << port_ << std::endl;
}

void Server::stop()
{
    running_ = false;

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

void Server::loop_()
{
    running_ = true;
    while(running_)
    {
        zmq::message_t request;
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
            pb_response.set_message("ERR_SERVER_EXCEPTION");
        }

        std::string binary_payload;
        bool success = pb_response.SerializeToString(&binary_payload);
        if (!success) {
            std::cerr << "[ERROR] Failed to serialize response" << std::endl;
            continue;
        }

        zmq::message_t reply(binary_payload.size());
        memcpy(reply.data(), binary_payload.data(), binary_payload.size());
        socket_.send(reply, zmq::send_flags::none);
    }
}

void Server::robot_worker_loop_() {
    while (running_) {
        // wait for a task to be added from the server thread
        auto next_id = tasker_.waitForNextTask();

        if (!next_id) break; // this means the server is shutting down

        task_id_t task_id = *next_id;

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
}

protocol::ServerResponse Server::process_request(const commands::DecodedRequest& decoded)
{
    protocol::ServerResponse response;

    switch (decoded.cmd_type)
    {
    case commands::RapidCommandType::UNKNOWN:
        response.set_status(protocol::ResponseStatus::ERROR);
        response.set_message("Unknown command");
        break;

    case commands::RapidCommandType::EXIT:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("OKISUWU");
        running_ = false;
        break;

    case commands::RapidCommandType::PING:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("PONGUWU");
        break;

    case commands::RapidCommandType::PINGR:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("PONGR_NOT_IMPLEMENTED");
        break;

    case commands::RapidCommandType::CHECK_TASK:
    {
        task_id_t id = decoded.task_id.value();

        auto task = tasker_.getTask(id);

        if (!task) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_message("FAILED_OR_NOT_FOUND");
            break;
        }

        TaskState state = task->getState();
        const std::string& message = task->getResponse();

        if (state == TaskState::COMPLETED) {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_message(message); // Create this getter too
            tasker_.removeTask(id); // Clean up memory once client gets the result
        }
        else if (state == TaskState::IN_PROGRESS || state == TaskState::PENDING) {
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_message("STILL_RUNNING");
        }
        else {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_message("FAILED_OR_NOT_FOUND");
        }
        break;
    }

    default:
        if (robots_.empty() || !robots_[0]->is_connected()) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_message("Robot is disconnected");
            break;
        }

        const commands::RapidRequest robot_req = create_rapid_request(decoded);
        task_id_t new_task_id = tasker_.addTask(robot_req);

        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message(std::to_string(new_task_id));
        break;
    }

    return response;
}

} // namespace robert::server
