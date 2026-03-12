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
    SET_ZONE     = 0x05,
    EXIT         = 0x06,
    PING         = 0x07,
    UNKNOWN      = 0xFF
};

#pragma pack(push, 1)
/**
 * @struct MessageCommand
 * @brief 
 * command_id (1 byte)
 * target (48 bytes)
 * target2 (48 bytes, only for MoveC)
 * joints ()
 */
struct MessageCommand {
    uint8_t command_id;
    BinRobTarget target;   // Used for L, J, and AbsJ (via ext_joint field)
    BinRobTarget target2;  // Used only for the second point in MoveC
    BinRobJoint joints;     // Used for MoveAbsJ to specify joint angles
    BinRobJoint ext_joints; // Used for specify external joint angles
    uint32_t speed;
    uint8_t zone;
};

#pragma pack(pop)

struct DecodedCommand {
    CommandType type{CommandType::UNKNOWN};
    RobTarget* target; // For L, J
    RobTarget* target2; // For C (second point)
    RobJoint* joints; // For MoveAbsJ, we can store the joint values here
    RobJoint* ext_joints; // For MoveAbsJ, we can store the external joint values here
    double speed{0.0};
    std::string precision;
};

std::string full_command_string(const DecodedCommand& cmd);

MessageCommand create_binary_message(const DecodedCommand& decoded);

} // namespace robert