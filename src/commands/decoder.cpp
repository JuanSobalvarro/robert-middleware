#include "commands/decoder.hpp"
#include "commands/commands.hpp"

#include <iostream>


namespace robert::commands {

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
    case protocol::CommandType::GETSTATUS:
        return RapidCommandType::GETSTATUS;
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

data::JointTargetBridge Decoder::translate_jointtarget(const protocol::JointTarget& target) {
    return data::JointTargetBridge(
        {target.robjoint().rax_1(), target.robjoint().rax_2(), target.robjoint().rax_3(), target.robjoint().rax_4(), target.robjoint().rax_5(), target.robjoint().rax_6()},
        {target.extjoint().eax_a(), target.extjoint().eax_b(), target.extjoint().eax_c(), target.extjoint().eax_d(), target.extjoint().eax_e(), target.extjoint().eax_f()}
    );
}

data::RobTargetBridge Decoder::translate_robtarget(const protocol::RobTarget& target) {
    return data::RobTargetBridge(
        data::RobPosition(target.trans().x(), target.trans().y(), target.trans().z()),
        data::RobOrientation(target.rot().q1(), target.rot().q2(), target.rot().q3(), target.rot().q4()),
        data::RobConfigData(target.robconf().cf1(), target.robconf().cf4(), target.robconf().cf6(), target.robconf().cfx()),
        data::RobJoint(target.extax().eax_a(), target.extax().eax_b(), target.extax().eax_c(), target.extax().eax_d(), target.extax().eax_e(), target.extax().eax_f())
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
        std::cout << "[DECODER] Target: " << drequest.target->to_string() << std::endl;
    }

    if (crequest.has_extra_target()) {
        drequest.extra_target = translate_robtarget(crequest.extra_target());
        std::cout << "[DECODER] Extra Target: " << drequest.extra_target->to_string() << std::endl;
    }

    if (crequest.has_joint_target()) {
        drequest.joint_target = translate_jointtarget(crequest.joint_target());
        std::cout << "[DECODER] Joints: " << drequest.joint_target->to_string() << std::endl;
    }

    drequest.speed = crequest.speed();

    drequest.zone = proto_zone_enum2rapid_zone_enum(crequest.zone());

    std::cout << "[DECODER] Speed: " << drequest.speed << ", Zone: " << type_to_string(static_cast<RapidCommandType>(drequest.zone)) << std::endl;

    return drequest;
}

template<typename T>
inline T read_from_buffer(const std::vector<uint8_t>& buffer, size_t offset) {
    T value;
    std::memcpy(&value, buffer.data() + offset, sizeof(T));
    return value;
}

bool Decoder::unpack_robot_status(const std::vector<uint8_t>& raw_data, protocol::RobotStatus* pb_state) {
    // Verificamos el tamaño mínimo esperado (193 bytes)
    if (raw_data.size() < 162) {
        std::cerr << "[DECODER] Telemetry payload too small: " << raw_data.size() << " bytes." << std::endl;
        return false;
    }

    pb_state->set_op_mode(static_cast<protocol::OpMode>(read_from_buffer<uint8_t>(raw_data, 0)));
    pb_state->set_speed_override(read_from_buffer<float>(raw_data, 1));
    pb_state->set_current_speed(read_from_buffer<float>(raw_data, 5));
    pb_state->set_current_zone(static_cast<protocol::Zone>(read_from_buffer<float>(raw_data, 9)));

    protocol::RobTarget* pb_target = pb_state->mutable_current_target();

    protocol::Position* pb_pos = pb_target->mutable_trans();
    pb_pos->set_x(read_from_buffer<float>(raw_data, 13));
    pb_pos->set_y(read_from_buffer<float>(raw_data, 17));
    pb_pos->set_z(read_from_buffer<float>(raw_data, 21));

    protocol::Orientation* pb_rot = pb_target->mutable_rot();
    pb_rot->set_q1(read_from_buffer<float>(raw_data, 25));
    pb_rot->set_q2(read_from_buffer<float>(raw_data, 29));
    pb_rot->set_q3(read_from_buffer<float>(raw_data, 33));
    pb_rot->set_q4(read_from_buffer<float>(raw_data, 37));

    protocol::ConfData* pb_conf = pb_target->mutable_robconf();
    pb_conf->set_cf1(read_from_buffer<int32_t>(raw_data, 41));
    pb_conf->set_cf4(read_from_buffer<int32_t>(raw_data, 45));
    pb_conf->set_cf6(read_from_buffer<int32_t>(raw_data, 49));
    pb_conf->set_cfx(read_from_buffer<int32_t>(raw_data, 53));

    protocol::ExtJoint* pb_extax = pb_target->mutable_extax();
    pb_extax->set_eax_a(read_from_buffer<float>(raw_data, 57));
    pb_extax->set_eax_b(read_from_buffer<float>(raw_data, 61));
    pb_extax->set_eax_c(read_from_buffer<float>(raw_data, 65));
    pb_extax->set_eax_d(read_from_buffer<float>(raw_data, 69));
    pb_extax->set_eax_e(read_from_buffer<float>(raw_data, 73));
    pb_extax->set_eax_f(read_from_buffer<float>(raw_data, 77));

    protocol::JointTarget* pb_joints = pb_state->mutable_current_joint_target();

    protocol::RobJoint* pb_robax = pb_joints->mutable_robjoint();
    pb_robax->set_rax_1(read_from_buffer<float>(raw_data, 81));
    pb_robax->set_rax_2(read_from_buffer<float>(raw_data, 85));
    pb_robax->set_rax_3(read_from_buffer<float>(raw_data, 89));
    pb_robax->set_rax_4(read_from_buffer<float>(raw_data, 93));
    pb_robax->set_rax_5(read_from_buffer<float>(raw_data, 97));
    pb_robax->set_rax_6(read_from_buffer<float>(raw_data, 101));

    protocol::ExtJoint* pb_extjoint = pb_joints->mutable_extjoint();
    pb_extjoint->set_eax_a(read_from_buffer<float>(raw_data, 105));
    pb_extjoint->set_eax_b(read_from_buffer<float>(raw_data, 109));
    pb_extjoint->set_eax_c(read_from_buffer<float>(raw_data, 113));
    pb_extjoint->set_eax_d(read_from_buffer<float>(raw_data, 117));
    pb_extjoint->set_eax_e(read_from_buffer<float>(raw_data, 121));
    pb_extjoint->set_eax_f(read_from_buffer<float>(raw_data, 125));

    char str_buf[32];
    std::memcpy(str_buf, raw_data.data() + 129, 32);
    pb_state->set_robot_time(std::string(str_buf));

    std::memcpy(str_buf, raw_data.data() + 161, 32);
    pb_state->set_robot_date(std::string(str_buf));

    return true;
}

//
} // namespace robert
