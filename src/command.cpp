#include "command.hpp"
#include <sstream>
#include <iomanip>
#include <span>
#include <cstring>
#include <format>

namespace robert
{

std::string full_command_string(const DecodedCommand& cmd)
{
    std::stringstream ss;

    ss << "[" << type_to_string(cmd.type) << "] ";

    if (!cmd.target.has_value()) {
        ss << "(No target data) ";
    }

    if (cmd.target.has_value() && cmd.target->to_string() != "") {
        ss << "T1: " << cmd.target->to_string() << " ";
    }
    
    if (cmd.type == CommandType::MOVE_C && cmd.target2.has_value() && cmd.target2->to_string() != "") {
        ss << "T2: " << cmd.target2->to_string() << " ";
    }

    return ss.str();
}

/**
 * @brief Logic for binary packing. 
 * This is the heart of the new byte-offset protocol.
 */
MessageCommand create_binary_message(const DecodedCommand& decoded)
{
    MessageCommand msg{}; // please remember to always zeroed a structure before putting data 
    std::memset(&msg, 0, sizeof(MessageCommand)); // this is just to be extra sure, we don't want any uninitialized data
    msg.command_id = static_cast<uint8_t>(decoded.type);

    if (decoded.type == CommandType::UNKNOWN) {
        return msg;
    }

    if (decoded.type == CommandType::MOVE_L || 
        decoded.type == CommandType::MOVE_J || 
        decoded.type == CommandType::MOVE_C) 
    {
        if (decoded.target.has_value()) {
            msg.target = decoded.target->to_bin();
        } else {
            throw std::invalid_argument("MoveL, MoveJ, and MoveC commands require target data.");
        }
    }

    if (decoded.type == CommandType::MOVE_C) {
        if (decoded.target2.has_value()) {
            msg.target2 = decoded.target2->to_bin();
        } else {
            throw std::invalid_argument("MoveC command requires a second target.");
        }
    }
    
    if (decoded.type == CommandType::MOVE_ABS_J) {
        if (!decoded.joints.has_value()) {
            throw std::invalid_argument("MoveAbsJ command requires joint data.");
        }
        msg.joints = decoded.joints->to_bin();

        if (decoded.ext_joints.has_value()) {
            msg.ext_joints = decoded.ext_joints->to_bin();
        }
    }

    if (decoded.type == CommandType::SET_SPEED) {
        msg.speed = static_cast<uint16_t>(decoded.speed);
    }

    if (decoded.type == CommandType::SET_ZONE) {
        msg.zone = static_cast<uint8_t>(decoded.zone);
    }

    return msg;
}

std::string message_command_to_string(const MessageCommand& msg) {
    std::stringstream ss;
    ss << "Command ID: " << static_cast<int>(msg.command_id) << " (" << type_to_string(static_cast<CommandType>(msg.command_id)) << ")\n";
    ss << "Target 1: " << msg.target.position.x << ", " << msg.target.position.y << ", " << msg.target.position.z << "\n";
    ss << "Target 2: " << msg.target2.position.x << ", " << msg.target2.position.y << ", " << msg.target2.position.z << "\n";
    ss << "Orientation: " << msg.target.orientation.q1 << ", " << msg.target.orientation.q2 << ", " << msg.target.orientation.q3 << ", " << msg.target.orientation.q4 << "\n";
    ss << "Config Data: " << msg.target.config_data.cf1 << ", " << msg.target.config_data.cf4 << ", " << msg.target.config_data.cf6 << ", " << msg.target.config_data.cfx << "\n";
    ss << "Joints: " << msg.joints.j1 << ", " << msg.joints.j2 << ", " << msg.joints.j3 << ", "
       << msg.joints.j4 << ", " << msg.joints.j5 << ", " << msg.joints.j6 << "\n";
    ss << "External Joints: " << msg.target.ext_joint.j1 << ", " << msg.target.ext_joint.j2 << ", " << msg.target.ext_joint.j3 << ", "
       << msg.target.ext_joint.j4 << ", " << msg.target.ext_joint.j5 << ", " << msg.target.ext_joint.j6 << "\n";
    ss << "Speed: " << msg.speed << "\n";
    ss << "Zone id: " << static_cast<int>(msg.zone) << "\n";

    return ss.str();
}

// 
std::string message_command_to_hexstring(const MessageCommand& msg) {
    auto bytes = std::span<const unsigned char>(
        reinterpret_cast<const unsigned char*>(&msg), sizeof(MessageCommand));
    
    std::string hexStr;
    hexStr.reserve(bytes.size() * 2); // Optimization: pre-allocate memory

    for (unsigned char b : bytes) {
        // {:02x} means: 0-padded, 2-width, lowercase hex
        hexStr += std::format("{:02x}", b);
    }
    return hexStr;
}

CommandType string_to_type(const std::string& cmd_str) {
    if (cmd_str == "MOVEL")    return CommandType::MOVE_L;
    if (cmd_str == "MOVEJ")    return CommandType::MOVE_J;
    if (cmd_str == "MOVEC")    return CommandType::MOVE_C;
    if (cmd_str == "MOVEABSJ") return CommandType::MOVE_ABS_J;
    if (cmd_str == "SETSPEED") return CommandType::SET_SPEED;
    if (cmd_str == "SETZONE") return CommandType::SET_ZONE;
    if (cmd_str == "EXIT")     return CommandType::EXIT;
    if (cmd_str == "PING")     return CommandType::PING;
    if (cmd_str == "PINGR")    return CommandType::PINGR;
    if (cmd_str == "ZERO")     return CommandType::ZERO;
    return CommandType::UNKNOWN;
}

std::string type_to_string(CommandType type) {
    switch (type) {
        case CommandType::MOVE_L: return "MOVEL";
        case CommandType::MOVE_J: return "MOVEJ";
        case CommandType::MOVE_C: return "MOVEC";
        case CommandType::MOVE_ABS_J: return "MOVEABSJ";
        case CommandType::SET_SPEED: return "SETSPEED";
        case CommandType::SET_ZONE: return "SETZONE";
        case CommandType::EXIT: return "EXIT";
        case CommandType::PING: return "PING";
        case CommandType::PINGR: return "PINGR";
        case CommandType::ZERO: return "ZERO";
        default: return "UNKNOWN";
    }
}

//
} // namespace robert