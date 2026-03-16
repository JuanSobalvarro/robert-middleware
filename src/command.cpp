#include "command.hpp"
#include <sstream>
#include <iomanip>

namespace robert
{

std::string cmd_to_name(CommandType type)
{
    switch (type)
    {
        case CommandType::MOVE_L: return "MoveL";
        case CommandType::MOVE_J: return "MoveJ";
        case CommandType::MOVE_C: return "MoveC";
        case CommandType::MOVE_ABS_J: return "MoveAbsJ";
        case CommandType::SET_SPEED: return "SetSpeed";
        case CommandType::SET_ZONE: return "SetZone";
        case CommandType::EXIT: return "Exit";
        case CommandType::PING: return "Ping";
        default: return "Unknown";
    }
}

std::string full_command_string(const DecodedCommand& cmd)
{
    std::stringstream ss;

    ss << "[" << cmd_to_name(cmd.type) << "] ";

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
    MessageCommand msg{};
    msg.command_id = static_cast<uint8_t>(decoded.type);

    if (decoded.type == CommandType::UNKNOWN) {
        return msg;
    }

    if (!decoded.target.has_value() && (decoded.type == CommandType::MOVE_L || decoded.type == CommandType::MOVE_J)) {
        throw std::invalid_argument("DecodedCommand should have minimum target data.");
    }

    if (!decoded.target2.has_value() && decoded.type == CommandType::MOVE_C) {
        throw std::invalid_argument("MoveC command requires a second target.");
    }

    msg.target = decoded.target->to_bin();

    if (decoded.type == CommandType::MOVE_C) {
        msg.target2 = decoded.target2->to_bin();
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
        msg.speed = decoded.speed;
    }

    if (decoded.type == CommandType::SET_ZONE) {
        msg.zone = static_cast<uint8_t>(decoded.zone);
    }

    return msg;
}

} // namespace robert