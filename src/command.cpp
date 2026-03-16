#include "command.hpp"
#include <sstream>
#include <iomanip>

namespace robert
{

std::string full_command_string(const DecodedCommand& cmd)
{
    std::stringstream ss;
    
    auto cmd_to_name = [](CommandType type) -> std::string {
        switch (type) {
            case MOVE_L: return "MoveL";
            case MOVE_J: return "MoveJ";
            case MOVE_C: return "MoveC";
            case MOVE_ABS_J: return "MoveAbsJ";
            case PING: return "PING";
            case EXIT: return "EXIT";
            default: return "UNK";
        }
    };

    ss << "[" << cmd_to_name(cmd.type) << "] ";

    if (cmd.target == nullptr) {
        ss << "(No target data) ";
    }

    if (cmd.target != nullptr && cmd.target->to_string() != "") {
        ss << "T1: " << cmd.target->to_string() << " ";
    }
    
    if (cmd.type == CommandType::MOVE_C && cmd.target2 != nullptr && cmd.target2->to_string() != "") {
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

    if (decoded.target == nullptr) {
        throw std::invalid_argument("DecodedCommand should have minimum target data.");
    }

    if (decoded.target2 == nullptr && decoded.type == CommandType::MOVE_C) {
        throw std::invalid_argument("MoveC command requires a second target.");
    }

    msg.target = decoded.target->to_bin();

    if (decoded.type == CommandType::MOVE_C) {
        msg.target2 = decoded.target2->to_bin();
    }
    
    if (decoded.type == CommandType::MOVE_ABS_J) {
        if (decoded.joints == nullptr) {
            throw std::invalid_argument("MoveAbsJ command requires joint data.");
        }
        msg.joints = decoded.joints->to_bin();

        if (decoded.ext_joints != nullptr) {
            msg.ext_joints = decoded.ext_joints->to_bin();
        }
    }

    return msg;
}

} // namespace robert