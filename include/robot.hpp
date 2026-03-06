#pragma once
#include <string>
#include <queue>

namespace robert
{

class Robot
{
public:
    Robot(const std::string& ip, int port);

    void start_session();
    void stop_session();

private:
    std::string ip_;
    int port_;

    std::queue<std::string> command_queue_;
};

} // namespace robert