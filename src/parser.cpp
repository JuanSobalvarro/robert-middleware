#include "parser.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace robert {

namespace {

std::string trim_copy(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }

    return text.substr(start, end - start);
}

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string part;

    while (std::getline(ss, part, delimiter)) {
        tokens.push_back(trim_copy(part));
    }

    return tokens;
}

bool parse_target_csv(const std::string& csv, RobTarget& out_target) {
    const std::vector<std::string> items = split(csv, ',');
    if (items.size() != 11) {
        return false;
    }

    std::vector<double> v;
    v.reserve(items.size());
    for (const auto& item : items) {
        v.push_back(std::stod(item));
    }

    RobPosition pos(v[0], v[1], v[2]);
    RobOrientation ori(v[3], v[4], v[5], v[6]);
    RobConfigData conf(static_cast<int32_t>(v[7]),
                       static_cast<int32_t>(v[8]),
                       static_cast<int32_t>(v[9]),
                       static_cast<uint8_t>(v[10]));
    // RobJoint ext_joint(v[11], v[12], v[13], v[14], v[15], v[16]);
    RobJoint ext_joint; // default to 9E9 for all joints, meaning not used in RAPID

    out_target = {pos, ori, conf, ext_joint};
    return true;
}

bool parse_joints_csv(const std::string& csv, RobJoint& out_joints) {

    const std::vector<std::string> items = split(csv, ',');

    if (items.size() != 6) {
        return false;
    }

    out_joints = {
        std::stof(items[0]), 
        std::stof(items[1]), 
        std::stof(items[2]), 
        std::stof(items[3]), 
        std::stof(items[4]), 
        std::stof(items[5])
    };

    std::cout << "[Parser] Parsed joints successfully" << std::endl;

    return true;
}

} // namespace

/**
 * @brief Parses a raw string from ZeroMQ into a DecodedCommand.
 * Expected format: "COMMAND|x,y,z,q1,q2,q3,q4,cf1,cf4,cf6,cfx,ext_j1,ext_j2,ext_j3,ext_j4,ext_j5,ext_j6"
 * For MoveC: "MOVEC|x1,y1,z1,q11,q12,q13,q14,cf11,cf14,cf16,cfx1,ext_j11,ext_j12,ext_j13,ext_j14,ext_j15,ext_j16|x2,y2,z2,q21,q22,q23,q24,cf21,cf24,cf26,cfx2,ext_j21,ext_j22,ext_j23,ext_j24,ext_j25,ext_j26"
 * For MoveAbsJ: "MOVEABSJ|j1,j2,j3,j4,j5,j6,ext_j1,ext_j2,ext_j3,ext_j4,ext_j5,ext_j6"
 */
DecodedCommand Parser::parse_string(const std::string& raw_msg) {
    DecodedCommand decoded;

    const std::vector<std::string> parts = split(raw_msg, '|');

    if (parts.empty()) return decoded;

    decoded.type = string_to_type(parts[0]);

    try {
        switch (decoded.type) {
            case CommandType::MOVE_L:
            case CommandType::MOVE_J: {
                if (parts.size() != 2) {
                    std::cout << "[Parser] Invalid parameter count for MOVE_L/MOVE_J: " << parts.size() - 1 << std::endl;
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                RobTarget target;
                if (!parse_target_csv(parts[1], target)) {
                    std::cout << "[Parser] Failed to parse target CSV for MOVE_L/MOVE_J" << std::endl;
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                decoded.target = target;
                break;
            }

            case CommandType::MOVE_C: {
                if (parts.size() != 3) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }

                RobTarget cir_target;
                RobTarget to_target;
                if (!parse_target_csv(parts[1], cir_target) || !parse_target_csv(parts[2], to_target)) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }

                decoded.target = cir_target;
                decoded.target2 = to_target;
                break;
            }

            case CommandType::MOVE_ABS_J: {
                if (parts.size() != 2) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }

                RobJoint joints;
                if (!parse_joints_csv(parts[1], joints)) {
                    decoded.type = CommandType::UNKNOWN;
                }
                decoded.joints = joints;
                break;
            }

            case CommandType::SET_SPEED: {
                if (parts.size() != 2) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                decoded.speed = std::stof(parts[1]);
                break;
            }

            case CommandType::SET_ZONE:
                if (parts.size() != 2 || parts[1].empty()) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                decoded.zone = std::stoi(parts[1]);
                break;

            case CommandType::EXIT:
            case CommandType::PING:
                break;

            default:
                std::cout << "[Parser] Unknown command type: " << parts[0] << std::endl;
                decoded.type = CommandType::UNKNOWN;
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Parser Error] Failed to decode parameters: " << e.what() << std::endl;
        decoded.type = CommandType::UNKNOWN;
    }

    return decoded;
}

CommandType Parser::string_to_type(const std::string& cmd_str) {
    if (cmd_str == "MOVEL")    return CommandType::MOVE_L;
    if (cmd_str == "MOVEJ")    return CommandType::MOVE_J;
    if (cmd_str == "MOVEC")    return CommandType::MOVE_C;
    if (cmd_str == "MOVEABSJ") return CommandType::MOVE_ABS_J;
    if (cmd_str == "SETSPEED") return CommandType::SET_SPEED;
    if (cmd_str == "SETZONE") return CommandType::SET_ZONE;
    if (cmd_str == "EXIT")     return CommandType::EXIT;
    if (cmd_str == "PING")     return CommandType::PING;
    return CommandType::UNKNOWN;
}

} // namespace robert