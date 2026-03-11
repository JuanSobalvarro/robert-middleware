#include "command.hpp"
#include <sstream>
#include <iomanip>

namespace robert
{

namespace {

std::string format_double(double value, int precision = 6)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    std::string out = ss.str();

    while (!out.empty() && out.back() == '0') {
        out.pop_back();
    }

    if (!out.empty() && out.back() == '.') {
        out.pop_back();
    }

    if (out == "-0") {
        return "0";
    }

    return out.empty() ? "0" : out;
}

} // namespace

std::string CommandFactory::command_string(const CommandType& ctype)
{
    switch (ctype)
    {
        case MOVE_L:        return "ML";
        case MOVE_J:        return "MJ";
        case MOVE_C:        return "MC";
        case MOVE_ABS_J:    return "MA";
        case SET_SPEED:     return "SS";
        case SET_PRECISION: return "SP";
        case STOP:          return "ST";
        case EXIT:          return "EXIT";
        default:            return "";
    }
}

std::vector<std::string> CommandFactory::command_strings()
{
    return {
        command_string(MOVE_L),
        command_string(MOVE_J),
        command_string(MOVE_C),
        command_string(MOVE_ABS_J),
        command_string(SET_SPEED),
        command_string(SET_PRECISION),
        command_string(STOP),
        command_string(EXIT)
    };
}

std::string CommandFactory::full_command_string(const CommandType& ctype,
                                                const std::vector<RobTarget>& targets,
                                                double speed,
                                                const std::string& precision,
                                                const std::array<double, 6>& joints)
{
    std::stringstream ss;

    const std::string cmd = command_string(ctype);
    if (cmd.empty())
    {
        return "";
    }

    ss << cmd << "|";

    switch (ctype)
    {
        case MOVE_L:
        case MOVE_J:
            if (targets.size() != 1)
            {
                return "";
            }
            ss << targets[0].to_string();
            break;

        case MOVE_C:
            if (targets.size() != 2)
            {
                return "";
            }
            ss << targets[0].to_string() << "|" << targets[1].to_string();
            break;

        case MOVE_ABS_J:
            ss << "[";
            for (std::size_t i = 0; i < joints.size(); ++i)
            {
                ss << format_double(joints[i], 4);
            }
            ss << "][9e9,9e9,9e9,9e9,9e9,9e9]";
            break;

        case SET_SPEED:
            ss << format_double(speed, 2);
            break;

        case SET_PRECISION:
            if (precision.empty())
            {
                return "";
            }
            ss << precision;
            break;

        case STOP:
            ss << "STOP";
            break;

        default:
            return "";
    }

    ss << ";";
    return ss.str();
}

} // namespace robert