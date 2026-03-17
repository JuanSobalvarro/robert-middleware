#pragma once

#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include <optional>

#include "target.hpp"
#include "values.hpp"

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
// !IMPORTANT ALWAYS FOLLOW 4 byte ALIGNMENT FOR ALL FIELDS IN THE STRUCTURES BELOW, OTHERWISE THE BINARY PROTOCOL WILL BE BROKEN
/**
 * @struct MessageCommand
 * @brief 
 * command_id (1 byte)
 * zone (1 byte, only for SetZone)
 * speed (2 bytes, only for SetSpeed)
 * target (68 bytes)
 * target2 (68 bytes, only for MoveC)
 * joints (24 bytes, only for MoveAbsJ)
 * ext_joints (24 bytes, only for MoveAbsJ)
 * Total size: 185 bytes
 */
struct MessageCommand {
    // first command, zone and speed to make sure we have a 4 byte alignment for the following float fields
    uint8_t command_id;
    uint8_t zone;
    uint16_t speed;
    BinRobTarget target;   // Used for L, J, and AbsJ (via ext_joint field)
    BinRobTarget target2;  // Used only for the second point in MoveC
    BinRobJoint joints;     // Used for MoveAbsJ to specify joint angles
    BinRobJoint ext_joints; // Used for specify external joint angles
};

#pragma pack(pop)

struct DecodedCommand {
    CommandType type{CommandType::UNKNOWN};
    std::optional<RobTarget> target; // For L, J
    std::optional<RobTarget> target2; // For C (second point)
    std::optional<RobJoint> joints; // For MoveAbsJ, we can store the joint values here
    std::optional<RobJoint> ext_joints; // For MoveAbsJ, we can store the external joint values here
    float speed{0.0};
    int zone;
};

std::string full_command_string(const DecodedCommand& cmd);

MessageCommand create_binary_message(const DecodedCommand& decoded);

std::string cmd_to_name(CommandType type);

std::string message_command_to_string(const MessageCommand& msg); // debugging function to visualize the binary message content

std::string message_command_to_hexstring(const MessageCommand& msg); // debugging function to visualize the raw bytes of the message

//
} // namespace robert