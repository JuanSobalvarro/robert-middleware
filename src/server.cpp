#include "server.hpp"

namespace robert
{

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
        // std::cout << "UWU" << std::endl;
        zmq::message_t request;

        // recv blocks until we receive a message
        zmq::recv_result_t res = socket_.recv(request, zmq::recv_flags::none);

        if (res.has_value() && (EAGAIN == res.value()))
        {
            throw std::runtime_error("[ERROR] There is an erorr with the recv");
        }
        
        std::string msg_str(static_cast<char*>(request.data()), request.size());
        std::cout << "[MIDDLEWARE] Received from Client: " << msg_str << std::endl;
        
        // TODO: MODULARIZE THIS SHIT

        std::string response = "ERR_UWUNKNOWN";
        
        try 
        {
            const DecodedCommand decoded = Parser::parse_string(msg_str);
            
            // why the switch? well there is special commands
            switch (decoded.type)
            {
            case CommandType::UNKNOWN:
                response = "ERR_PARSE";
                break;
            case CommandType::EXIT:
                response = "OKISUWU";
                running_ = false;
                break;
            case CommandType::PING:
                response = "PONGUWU";
                break;                
            // default are robot commands
            default:
                std::cout << "[DEBUG] Reached default command (moves)" << std::endl;
                if (robots_.empty()) {
                    response = "ERR_NOROBOT";
                    break;
                }

                if (robots_[0]->is_connected() == false) {
                    response = "ERR_ROBOTDISCONNECTED";
                    break;
                }

                const MessageCommand robot_command = create_binary_message(decoded);
                std::cout << "[DEBUG] Created binary message for robot: " << std::endl << message_command_to_string(robot_command) << std::endl;
                std::cout << "[DEBUG] Command as hex: " << message_command_to_hexstring(robot_command) << std::endl;
                const std::string robot_command_str = full_command_string(decoded);

                std::cout << "[C++ to Robot] Queueing: " << robot_command_str << std::endl;
                std::future<std::string> future_ack = robots_[0]->queue_message(robot_command);

                if (future_ack.wait_for(std::chrono::seconds(30)) != std::future_status::ready) {
                    response = "ERR_TIMEOUT";
                    break;
                }
                
                response = future_ack.get();
                std::cout << "[Robot ACK] " << response << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[SERVER ERROR] " << e.what() << std::endl;
            response = "ERR_UNKNOWN";
        }

        zmq::message_t reply(response.size());
        memcpy(reply.data(), response.data(), response.size());
        socket_.send(reply, zmq::send_flags::none);
    }
}

} // namespace robert
