#include "command.hpp"

std::string robert::CommandFactory::command_string(const CommandType& ctype)
{
    switch (ctype)
    {
    case MOVEL:
        return "MOVEL";
    case MOVEJ:
        return "MOVEJ";
    case MOVEABSJ:
        return "MOVEABSJ";
    case MOVEC:
        return "MOVEC";
    case EXIT:
        return "EXIT";
    default:
        return "UNKNOWN";
    }
}

std::array<std::string, 5> robert::CommandFactory::command_strings()
{
    return {
        command_string(MOVEL),
        command_string(MOVEJ),
        command_string(MOVEABSJ),
        command_string(MOVEC),
        command_string(EXIT)
    };
}

