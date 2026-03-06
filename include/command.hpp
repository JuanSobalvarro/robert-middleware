#include <string>
#include <array>
#include <vector>

namespace robert
{

enum CommandType
{
    MOVEL,
    MOVEJ,
    MOVEABSJ,
    MOVEC,
    EXIT,
};

class CommandFactory
{
public:

    std::string command_string(const CommandType& ctype);
    std::array<std::string, 5> command_strings();
    std::string full_command_string(const CommandType& ctype, const std::vector<double>& params);
};

} // namespace robert