#pragma once

#include <string>
#include <vector>
#include <netinet/in.h>

namespace robert
{

class Session
/*
* This class will manage the connection to the robot, it should only handle 1 session at a time, and it should be able to send commands to the robot.
* Also it should accept messages but automatically return response that is currently being used
*/
{
public:
    Session(const std::string& ip, int port);
    ~Session();

    void start();
    void stop();

private:
    std::string ip_;
    int port_;

    bool is_active_;
};

// 

} // namespace robert