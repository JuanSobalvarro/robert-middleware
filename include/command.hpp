#pragma once

#include <string>
#include <cstring>
#include <array>
#include <span>
#include <vector>
#include <format>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <optional>

#include "data.hpp"

namespace robert
{

enum RapidCommandType : uint8_t // Explicitly 1 byte
{
    MOVE_L       = 0x00,
    MOVE_J       = 0x01,
    MOVE_C       = 0x02,
    MOVE_ABS_J   = 0x03,
    SET_SPEED    = 0x04,
    SET_ZONE     = 0x05,
    EXIT         = 0x06,
    PING         = 0x07,
    PINGR        = 0x08,
    ZERO         = 0x09,
    UNKNOWN      = 0xFF
};

enum RapidZone {
    FINE = 0,
    Z1 = 1,
    Z5 = 2,
    Z10 = 3,
    Z15 = 4,
    Z20 = 5,
    Z30 = 6,
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
 * extra_target (68 bytes, only for MoveC)
 * joints (24 bytes, only for MoveAbsJ)
 * ext_joints (24 bytes, only for MoveAbsJ)
 * Total size: 185 bytes
 */
struct RapidRequest {
    // first command, zone and speed to make sure we have a 4 byte alignment for the following float fields
    uint8_t command_id;
    uint8_t zone;
    uint16_t speed;
    RapidRobTarget target;   // Used for L, J, and AbsJ (via ext_joint field)
    RapidRobTarget extra_target;  // Used only for the second point in MoveC
    RapidRobJoint joints;     // Used for MoveAbsJ to specify joint angles
    RapidRobJoint ext_joints; // Used for specify external joint angles
};

#pragma pack(pop)

struct DecodedRequest {
    RapidCommandType cmd_type{RapidCommandType::UNKNOWN};
    std::optional<RobTargetBridge> target; // For L, J
    std::optional<RobTargetBridge> extra_target; // For C (second point)
    std::optional<JointTargetBridge> joints; // For MoveAbsJ, we can store the joint values here
    std::optional<JointTargetBridge> ext_joints; // For MoveAbsJ, we can store the external joint values here
    float speed{0.0};
    RapidZone zone{RapidZone::FINE};
};

RapidRequest create_rapid_request(const DecodedRequest& decoded);

std::string rapid_request_to_string(const RapidRequest& msg); // debugging function to visualize the binary message content

std::string rapid_request_to_hexstring(const RapidRequest& msg); // debugging function to visualize the raw bytes of the message

RapidCommandType string_to_type(const std::string& cmd_str);
std::string type_to_string(RapidCommandType cmd_type);

//
} // namespace robert