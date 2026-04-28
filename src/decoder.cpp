#include "decoder.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace robert {

RapidCommandType Decoder::proto_cmd_enum2rapid_cmd_enum(protocol::CommandType cmd) {
    switch(cmd) {
    case protocol::CommandType::MOVEL:
        return RapidCommandType::MOVE_L;
    case protocol::CommandType::MOVEJ:
        return RapidCommandType::MOVE_J;
    case protocol::CommandType::MOVEC:
        return RapidCommandType::MOVE_C;
    case protocol::CommandType::MOVEABSJ:
        return RapidCommandType::MOVE_ABS_J;
    case protocol::CommandType::SETSPEED:
        return RapidCommandType::SET_SPEED;
    case protocol::CommandType::SETZONE:
        return RapidCommandType::SET_ZONE;
    case protocol::CommandType::EXIT:
        return RapidCommandType::EXIT;
    case protocol::CommandType::PING:
        return RapidCommandType::PING;
    case protocol::CommandType::PINGR:
        return RapidCommandType::PINGR;
    case protocol::CommandType::ZERO:
        return RapidCommandType::ZERO;
    default:
        return RapidCommandType::UNKNOWN;
    }
}

RapidZone Decoder::proto_zone_enum2rapid_zone_enum(protocol::Zone zone) {
    switch(zone) {
    case protocol::Zone::FINE:
        return RapidZone::FINE;
    case protocol::Zone::Z1:
        return RapidZone::Z1;
    case protocol::Zone::Z5:
        return RapidZone::Z5;
    case protocol::Zone::Z10:
        return RapidZone::Z10;
    case protocol::Zone::Z15:
        return RapidZone::Z15;
    case protocol::Zone::Z20:
        return RapidZone::Z20;
    case protocol::Zone::Z30:
        return RapidZone::Z30;
    default:
        return RapidZone::FINE; // default to FINE if unknown
    }
}

JointTargetBridge Decoder::translate_jointtarget(const protocol::JointTarget& target) {
    return JointTargetBridge(
        {target.robjoint().rax_1(), target.robjoint().rax_2(), target.robjoint().rax_3(), target.robjoint().rax_4(), target.robjoint().rax_5(), target.robjoint().rax_6()},
        {target.extjoint().eax_a(), target.extjoint().eax_b(), target.extjoint().eax_c(), target.extjoint().eax_d(), target.extjoint().eax_e(), target.extjoint().eax_f()}
    );
}

RobTargetBridge Decoder::translate_robtarget(const protocol::RobTarget& target) {
    return RobTargetBridge(
        RobPosition(target.trans().x(), target.trans().y(), target.trans().z()),
        RobOrientation(target.rot().q1(), target.rot().q2(), target.rot().q3(), target.rot().q4()),
        RobConfigData(target.robconf().cf1(), target.robconf().cf4(), target.robconf().cf6(), target.robconf().cfx()),
        RobJoint(target.extax().eax_a(), target.extax().eax_b(), target.extax().eax_c(), target.extax().eax_d(), target.extax().eax_e(), target.extax().eax_f())
    );
}

/**
 * @brief Parses a raw bytes message (protobuf) received from the client and decodes it into a DecodedRequest structure.
 */
DecodedRequest Decoder::decode_buffer(const std::string& raw_msg) {
    protocol::ClientRequest crequest;
    DecodedRequest drequest;

    if (!crequest.ParseFromString(raw_msg)) {
        std::cerr << "[DECODER] Failed to parse protobuf message from client" << std::endl;
        return drequest;
    }    

    drequest.cmd_type = proto_cmd_enum2rapid_cmd_enum(crequest.command());

    if (crequest.has_target()) {
        drequest.target = translate_robtarget(crequest.target());
    }

    if (crequest.has_extra_target()) {
        drequest.extra_target = translate_robtarget(crequest.extra_target());
    }

    if (crequest.has_joint_target()) {
        drequest.joints = translate_jointtarget(crequest.joint_target());
    }

    drequest.speed = crequest.speed();

    drequest.zone = proto_zone_enum2rapid_zone_enum(crequest.zone());

    return drequest;
}

//
} // namespace robert