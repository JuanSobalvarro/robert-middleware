#pragma once

#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include "target.hpp"

namespace robert
{

enum CommandType : uint8_t // Explicitly 1 byte
{
    MOVE_L       = 0x00,
    MOVE_J       = 0x01,
    MOVE_C       = 0x02,
    MOVE_ABS_J   = 0x03,
    SET_SPEED    = 0x04,
    SET_PRECISION= 0x05,
    STOP         = 0x06,
    EXIT         = 0x07,
    PING         = 0x08,
    UNKNOWN      = 0xFF
};

#pragma pack(push, 1)
/**
 * @struct MessageCommand
 * Total: 1 (ID) + 68 (T1) + 68 (T2) = 137 bytes.
 */
struct MessageCommand {
    uint8_t command_id;
    BinRobTarget target;   // Used for L, J, and AbsJ (via ext_joint field)
    BinRobTarget target2;  // Used only for the second point in MoveC
};
#pragma pack(pop)

// Data structure for internal middleware logic
struct DecodedCommand {
    CommandType type{CommandType::UNKNOWN};
    RobTarget target; // For L, J
    RobTarget target2; // For C (second point)
    RobJoint joints; // For MoveAbsJ, we can store the joint values here
    double speed{0.0};
    std::string precision;
};

class CommandFactory
{
public:
    // string version for debugging/logging
    std::string full_command_string(const DecodedCommand& cmd);

    MessageCommand create_binary_message(const DecodedCommand& decoded);
};

} // namespace robert