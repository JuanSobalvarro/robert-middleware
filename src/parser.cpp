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
    RobConfigData conf(static_cast<int>(v[7]),
                       static_cast<int>(v[8]),
                       static_cast<int>(v[9]),
                       static_cast<unsigned int>(v[10]));

    out_target = RobTarget(pos, ori, conf);
    return true;
}

bool parse_joints_csv(const std::string& csv, std::array<double, 6>& out_joints) {
    const std::vector<std::string> items = split(csv, ',');
    if (items.size() != 6) {
        return false;
    }

    for (std::size_t i = 0; i < out_joints.size(); ++i) {
        out_joints[i] = std::stod(items[i]);
    }

    return true;
}

} // namespace

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
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }

                RobTarget target;
                if (!parse_target_csv(parts[1], target)) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }

                decoded.targets.push_back(target);
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

                decoded.targets.push_back(cir_target);
                decoded.targets.push_back(to_target);
                break;
            }

            case CommandType::MOVE_ABS_J:
                if (parts.size() != 2 || !parse_joints_csv(parts[1], decoded.joints)) {
                    decoded.type = CommandType::UNKNOWN;
                }
                break;

            case CommandType::SET_SPEED:
                if (parts.size() != 2) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                decoded.speed = std::stod(parts[1]);
                break;

            case CommandType::SET_PRECISION:
                if (parts.size() != 2 || parts[1].empty()) {
                    decoded.type = CommandType::UNKNOWN;
                    break;
                }
                decoded.precision = parts[1];
                break;

            case CommandType::STOP:
            case CommandType::EXIT:
            case CommandType::PING:
                break;

            default:
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
    if (cmd_str == "SETPRECISION") return CommandType::SET_PRECISION;
    if (cmd_str == "STOP")     return CommandType::STOP;
    if (cmd_str == "EXIT")     return CommandType::EXIT;
    if (cmd_str == "PING")     return CommandType::PING;
    return CommandType::UNKNOWN;
}

} // namespace robert