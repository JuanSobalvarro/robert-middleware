#include "server.hpp"

robert::Server::Server(const std::string& ip, int port) 
    : ip_(ip), port_(port), context_(1), socket_(context_, ZMQ_REP)
{
    std::string address = "tcp://" + ip_ + ":" + std::to_string(port_);
    socket_.bind(address);
}

robert::Server::~Server()
{
    stop();
}  

void robert::Server::start()
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
    
    std::cout << "[MIDDLEWARE] ZeroMQ Server listening on " << ip_ << ":" << port_ << std::endl;
}

void robert::Server::stop()
{
    running_ = false;

    if (server_thread_.joinable())
    {
        server_thread_.join();
    }
    
    for (auto& robot : robots_)
    {
        robot->stop_session();
    }
    
    std::cout << "[MIDDLEWARE] Server stopped and all robot sessions closed." << std::endl;
}

void robert::Server::load_robots_from_file(const std::string& filepath)
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

            robots_.push_back(std::make_unique<Robot>(ip, port));
        }
    }

}

void robert::Server::loop_()
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
            const DecodedRequest decoded = Decoder::decode_buffer(buffer);
            
            pb_response = process_request(decoded);
        }
        catch (const std::exception& e) {
            std::cerr << "[SERVER ERROR] " << e.what() << std::endl;
            pb_response.set_status(protocol::ResponseStatus::ERROR);
            pb_response.set_message("ERR_SERVER_EXCEPTION");
        }

        std::string binary_payload;
        pb_response.SerializeToString(&binary_payload);

        zmq::message_t reply(binary_payload.size());
        memcpy(reply.data(), binary_payload.data(), binary_payload.size());
        socket_.send(reply, zmq::send_flags::none);
    }
}

robert::protocol::ServerResponse robert::Server::process_request(const DecodedRequest& decoded)
{
    protocol::ServerResponse response;

    switch (decoded.cmd_type)
    {
    case RapidCommandType::UNKNOWN:
        response.set_status(protocol::ResponseStatus::ERROR);
        response.set_message("Unknown command");
        break;

    case RapidCommandType::EXIT:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("OKISUWU");
        running_ = false; 
        break;

    case RapidCommandType::PING:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("PONGUWU");
        break;

    case RapidCommandType::PINGR:
        response.set_status(protocol::ResponseStatus::SUCCESS);
        response.set_message("PONGR_NOT_IMPLEMENTED");
        break;

    default:
        if (robots_.empty() || !robots_[0]->is_connected()) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_message("Robot is disconnected or not available");
            break;
        }

        const RapidRequest robot_req = create_rapid_request(decoded);
        std::future<std::vector<uint8_t>> future_ack = robots_[0]->queue_request(robot_req);

        if (future_ack.wait_for(std::chrono::seconds(35)) != std::future_status::ready) {
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_message("Timeout");
            break;
        }
        
        std::vector<uint8_t> raw_response = future_ack.get();

        if (decoded.cmd_type == RapidCommandType::GETSTATUS) {
            protocol::RobotStatus* pb_state = response.mutable_robot_status();
            
            if (Decoder::unpack_robot_status(raw_response, pb_state)) {
                response.set_status(protocol::ResponseStatus::SUCCESS);
                response.set_message("STATUS_OK");
            } else {
                response.set_status(protocol::ResponseStatus::ERROR);
                std::string err_msg(raw_response.begin(), raw_response.end());
                response.set_message(err_msg.empty() ? "ERR_INCOMPLETE_TELEMETRY" : err_msg);
            }
        } 
        else {
            std::string robot_ack(raw_response.begin(), raw_response.end());
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_message(robot_ack);
        }
        break;
    }

    return response;
}

