#pragma once

#include <string>
#include <array>
#include <vector>
#include <sstream>
#include "target.hpp"
#include "command.hpp"
#include "values.hpp"

namespace robert {

class Parser {
public:
    /**
     * @brief Parses a raw string from ZeroMQ into a DecodedCommand.
     * Expected format: "COMMAND|x,y,z,q1,q2,q3,q4,cf1,cf4,cf6,cfx"
     */
    static DecodedCommand parse_string(const std::string& raw_msg);
};

} // namespace robert