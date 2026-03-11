#include "command.hpp"
#include <sstream>
#include <iomanip>

namespace robert
{

// --- Helper for Logging (Internal Namespace) ---
namespace {
    std::string format_double(double value, int precision = 4) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    }
}

// Re-implemented to support the new DecodedCommand struct
std::string CommandFactory::full_command_string(const DecodedCommand& cmd)
{
    std::stringstream ss;
    
    // Mapping Enum to human-readable strings for logs
    auto cmd_to_name = [](CommandType type) -> std::string {
        switch (type) {
            case MOVE_L: return "ML";
            case MOVE_J: return "MJ";
            case MOVE_C: return "MC";
            case MOVE_ABS_J: return "MA";
            case PING: return "PING";
            case EXIT: return "EXIT";
            default: return "UNK";
        }
    };

    ss << "[" << cmd_to_name(cmd.type) << "] ";

    if (!cmd.targets.empty()) {
        // Here we use the to_string() you defined in RobTarget
        ss << "T1: " << cmd.targets[0].to_string();
    }
    
    if (cmd.targets.size() > 1) {
        ss << " | T2: " << cmd.targets[1].to_string();
    }

    if (cmd.type == SET_SPEED) {
        ss << "Speed: " << cmd.speed;
    }

    return ss.str();
}

/**
 * @brief Logic for binary packing. 
 * This is the heart of the new byte-offset protocol.
 */
MessageCommand CommandFactory::create_binary_message(const DecodedCommand& decoded)
{
    MessageCommand msg{};
    msg.command_id = static_cast<uint8_t>(decoded.type);

    // Populate Target 1 (Used by almost all motion commands)
    if (!decoded.targets.empty()) {
        msg.target = decoded.targets[0].to_bin();
    } else if (decoded.type == MOVE_ABS_J) {
        // For MoveAbsJ, we don't send coordinates, but the joint values 
        // are stored in the ext_joint field of the robtarget structure.
        // We use a default/empty target and just fill the joints.
        RobJoint joints_data;
        // Since decoded.joints isn't in your DecodedCommand anymore, 
        // ensure your logic populates a RobTarget with joints for MA.
    }

    // Populate Target 2 (Specific for Circular Motion)
    if (decoded.targets.size() > 1) {
        msg.target2 = decoded.targets[1].to_bin();
    }

    return msg;
}

} // namespace robert