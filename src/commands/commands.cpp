#include "commands.hpp"

#include <string>
#include <cstring>
#include <span>
#include <stdexcept>
#include <sstream>
#include <format>


namespace robert::commands
{

RapidRequest create_rapid_request(const DecodedRequest& decoded)
{
    RapidRequest msg{}; // please remember to always zeroed a structure before putting data
    std::memset(&msg, 0, sizeof(RapidRequest)); // this is just to be extra sure, we don't want any uninitialized data
    msg.command_id = static_cast<uint8_t>(decoded.cmd_type);

    if (decoded.cmd_type == RapidCommandType::UNKNOWN) {
        return msg;
    }

    if (decoded.cmd_type == RapidCommandType::MOVE_L ||
        decoded.cmd_type == RapidCommandType::MOVE_J ||
        decoded.cmd_type == RapidCommandType::MOVE_C)
    {
        if (decoded.target.has_value()) {
            msg.target = decoded.target->to_rapid();
        } else {
            throw std::invalid_argument("MoveL, MoveJ, and MoveC commands require target data.");
        }
    }

    if (decoded.cmd_type == RapidCommandType::MOVE_C) {
        if (decoded.extra_target.has_value()) {
            msg.extra_target = decoded.extra_target->to_rapid();
        } else {
            throw std::invalid_argument("MoveC command requires a second target.");
        }
    }

    if (decoded.cmd_type == RapidCommandType::MOVE_ABS_J) {
        if (!decoded.joint_target.has_value()) {
            throw std::invalid_argument("MoveAbsJ command requires joint data.");
        }
        msg.joint_target = decoded.joint_target->to_rapid();
    }

    if (decoded.cmd_type == RapidCommandType::SET_SPEED) {
        msg.speed = static_cast<uint16_t>(decoded.speed);
    }

    if (decoded.cmd_type == RapidCommandType::SET_ZONE) {
        msg.zone = static_cast<uint8_t>(decoded.zone);
    }

    return msg;
}

std::string rapid_request_to_string(const RapidRequest& msg) {
    std::stringstream ss;
    ss << "Command ID: " << static_cast<int>(msg.command_id) << " (" << type_to_string(static_cast<RapidCommandType>(msg.command_id)) << ")\n";
    ss << "Target 1: " << msg.target.position.x << ", " << msg.target.position.y << ", " << msg.target.position.z << "\n";
    ss << "Target 2: " << msg.extra_target.position.x << ", " << msg.extra_target.position.y << ", " << msg.extra_target.position.z << "\n";
    ss << "Orientation: " << msg.target.orientation.q1 << ", " << msg.target.orientation.q2 << ", " << msg.target.orientation.q3 << ", " << msg.target.orientation.q4 << "\n";
    ss << "Config Data: " << msg.target.config_data.cf1 << ", " << msg.target.config_data.cf4 << ", " << msg.target.config_data.cf6 << ", " << msg.target.config_data.cfx << "\n";
    ss << "Joints: " << msg.joint_target.joints.j1 << ", " << msg.joint_target.joints.j2 << ", " << msg.joint_target.joints.j3 << ", "
       << msg.joint_target.joints.j4 << ", " << msg.joint_target.joints.j5<< ", " << msg.joint_target.joints.j6 << "\n";
    ss << "External Joints: " << msg.target.ext_joint.j1<< ", " << msg.target.ext_joint.j2<< ", " << msg.target.ext_joint.j3<< ", "
       << msg.target.ext_joint.j4<< ", " << msg.target.ext_joint.j5<< ", " << msg.target.ext_joint.j6<< "\n";
    ss << "Speed: " << msg.speed<< "\n";
    ss << "Zone id: " << static_cast<int>(msg.zone)<< "\n";

    return ss.str();
}

// Convert a RapidRequest to a hexadecimal string for debugging purposes
std::string rapid_request_to_hexstring(const RapidRequest& msg) {
    auto bytes = std::span<const unsigned char>(
        reinterpret_cast<const unsigned char*>(&msg), sizeof(RapidRequest));

    std::string hexStr;
    hexStr.reserve(bytes.size() * 2);

    for (unsigned char b : bytes) {
        // {:02x} means: 0-padded, 2-width, lowercase hex
        hexStr += std::format("{:02x}", b);
    }
    return hexStr;
}

RapidCommandType string_to_type(const std::string& cmd_str) {
    if (cmd_str == "MOVEL")    return RapidCommandType::MOVE_L;
    if (cmd_str == "MOVEJ")    return RapidCommandType::MOVE_J;
    if (cmd_str == "MOVEC")    return RapidCommandType::MOVE_C;
    if (cmd_str == "MOVEABSJ") return RapidCommandType::MOVE_ABS_J;
    if (cmd_str == "SETSPEED") return RapidCommandType::SET_SPEED;
    if (cmd_str == "SETZONE") return RapidCommandType::SET_ZONE;
    if (cmd_str == "EXIT")     return RapidCommandType::EXIT;
    if (cmd_str == "PING")     return RapidCommandType::PING;
    if (cmd_str == "PINGR")    return RapidCommandType::PINGR;
    if (cmd_str == "ZERO")     return RapidCommandType::ZERO;
    if (cmd_str == "GETSTATUS") return RapidCommandType::GETSTATUS;
    if (cmd_str == "CHECK_TASK") return RapidCommandType::CHECK_TASK;
    if (cmd_str == "LOGIN")    return RapidCommandType::LOGIN;
    if (cmd_str == "LOGOUT")   return RapidCommandType::LOGOUT;
    if (cmd_str == "ACQUIRE")  return RapidCommandType::ACQUIRE;
    if (cmd_str == "RELEASE")  return RapidCommandType::RELEASE;
    return RapidCommandType::UNKNOWN;
}

std::string type_to_string(RapidCommandType cmd_type) {
    switch (cmd_type) {
        case RapidCommandType::MOVE_L: return "MOVEL";
        case RapidCommandType::MOVE_J: return "MOVEJ";
        case RapidCommandType::MOVE_C: return "MOVEC";
        case RapidCommandType::MOVE_ABS_J: return "MOVEABSJ";
        case RapidCommandType::SET_SPEED: return "SETSPEED";
        case RapidCommandType::SET_ZONE: return "SETZONE";
        case RapidCommandType::EXIT: return "EXIT";
        case RapidCommandType::PING: return "PING";
        case RapidCommandType::PINGR: return "PINGR";
        case RapidCommandType::ZERO: return "ZERO";
        case RapidCommandType::GETSTATUS: return "GETSTATUS";
        case RapidCommandType::CHECK_TASK: return "CHECK_TASK";
        case RapidCommandType::LOGIN: return "LOGIN";
        case RapidCommandType::LOGOUT: return "LOGOUT";
        case RapidCommandType::ACQUIRE: return "ACQUIRE";
        case RapidCommandType::RELEASE: return "RELEASE";
        default: return "UNKNOWN";
    }
}

//
} // namespace robert
