#pragma once

#include <string>
#include <array>
#include <vector>
#include <functional>

#include "target.hpp"

namespace robert
{

enum CommandType
{
    MOVE_L,
    MOVE_J,
    MOVE_C,
    MOVE_ABS_J,
    SET_SPEED,
    SET_PRECISION,
    STOP,
    EXIT,
    PING,
    UNKNOWN
};

class CommandFactory
{
public:

    std::string command_string(const CommandType& ctype);
    std::vector<std::string> command_strings();
    std::string full_command_string(const CommandType& ctype,
                                    const std::vector<RobTarget>& targets = {},
                                    double speed = 0.0,
                                    const std::string& precision = "",
                                    const std::array<double, 6>& joints = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0});

};

/**
 * @struct DecodedCommand
 * @brief Holds the results of a parsed message from the Python client.
 */
struct DecodedCommand {
    CommandType type{CommandType::UNKNOWN};
    std::vector<RobTarget> targets;
    std::array<double, 6> joints{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double speed{0.0};
    std::string precision;
};

} // namespace robert