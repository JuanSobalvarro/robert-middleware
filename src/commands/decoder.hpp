#pragma once

#include <string>
#include <vector>

#include "core/data.hpp"
#include "commands/commands.hpp"

#include "protocol/protocol.pb.h"

namespace robert::commands {

class Decoder {
public:
    static RapidCommandType proto_cmd_enum2rapid_cmd_enum(protocol::CommandType cmd);
    static RapidZone proto_zone_enum2rapid_zone_enum(protocol::Zone zone);

    /**
     * @brief decode a protocol buffer.
     */
    static DecodedRequest decode_buffer(const std::string& raw_msg);

    static data::RobTargetBridge translate_robtarget(const protocol::RobTarget& target);

    static data::JointTargetBridge translate_jointtarget(const protocol::JointTarget& target);

    static bool unpack_robot_status(const std::vector<uint8_t>& raw_data, protocol::RobotStatus* pb_state);
};

} // namespace robert
