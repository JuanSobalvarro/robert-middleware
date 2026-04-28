#pragma once

#include <string>
#include <array>
#include <vector>
#include <sstream>

#include "data.hpp"
#include "command.hpp"

#include "protocol/protocol.pb.h"

namespace robert {

class Decoder {
public:
    static RapidCommandType proto_cmd_enum2rapid_cmd_enum(protocol::CommandType cmd);
    static RapidZone proto_zone_enum2rapid_zone_enum(protocol::Zone zone);

    /**
     * @brief decode a protocol buffer.
     */
    static DecodedRequest decode_buffer(const std::string& raw_msg);

    static RobTargetBridge translate_robtarget(const protocol::RobTarget& target);

    static JointTargetBridge translate_jointtarget(const protocol::JointTarget& target);
};

} // namespace robert