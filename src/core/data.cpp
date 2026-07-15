#include "data.hpp"

#include <cstring>
#include <sstream>

namespace robert::data
{

// --- RobPosition ---
RobPosition::RobPosition(float x, float y, float z) : position_{x, y, z} {}
RobPosition::RobPosition(const RobPosition& other) : position_(other.position_) {}
RobPosition::RobPosition() : position_{0.0f, 0.0f, 0.0f} {}
RobPosition::~RobPosition() {}

void RobPosition::set_position(float x, float y, float z) {
    position_ = {x, y, z};
}

RapidPosition RobPosition::to_rapid() const {
    RapidPosition bin;
    std::memcpy(&bin.x, &position_[0], 4);
    std::memcpy(&bin.y, &position_[1], 4);
    std::memcpy(&bin.z, &position_[2], 4);
    return bin;
}

// --- RobOrientation ---
RobOrientation::RobOrientation(float q1, float q2, float q3, float q4)
    : orientation_{q1, q2, q3, q4} {}
RobOrientation::RobOrientation(const RobOrientation& other) : orientation_(other.orientation_) {}
RobOrientation::RobOrientation() : orientation_{1.0f, 0.0f, 0.0f, 0.0f} {} // Identity quaternion
RobOrientation::~RobOrientation() {}

void RobOrientation::set_orientation(float q1, float q2, float q3, float q4) {
    orientation_ = {q1, q2, q3, q4};
}

RapidOrientation RobOrientation::to_rapid() const {
    RapidOrientation bin;
    std::memcpy(&bin.q1, &orientation_[0], 4);
    std::memcpy(&bin.q2, &orientation_[1], 4);
    std::memcpy(&bin.q3, &orientation_[2], 4);
    std::memcpy(&bin.q4, &orientation_[3], 4);
    return bin;
}

// --- RobConfigData ---
RobConfigData::RobConfigData(int cf1, int cf4, int cf6, unsigned int cfx)
    : config_data_{cf1, cf4, cf6, static_cast<int>(cfx)} {}
RobConfigData::RobConfigData(const RobConfigData& other) : config_data_(other.config_data_) {}
RobConfigData::RobConfigData() : config_data_{0, 0, 0, 0} {}
RobConfigData::~RobConfigData() {}

void RobConfigData::set_config_data(int cf1, int cf4, int cf6, unsigned int cfx) {
    config_data_ = {cf1, cf4, cf6, static_cast<int>(cfx)};
}

RapidConfData RobConfigData::to_rapid() const {
    RapidConfData bin;
    bin.cf1 = config_data_[0];
    bin.cf4 = config_data_[1];
    bin.cf6 = config_data_[2];
    bin.cfx = static_cast<uint32_t>(config_data_[3]);
    return bin;
}

RobJoint::RobJoint(float j1, float j2, float j3, float j4, float j5, float j6) : joints_{j1, j2, j3, j4, j5, j6} {}
RobJoint::RobJoint(const RobJoint& other) : joints_(other.joints_) {}
RobJoint::RobJoint() : joints_{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} {}
RobJoint::~RobJoint() {}
RapidRobJoint RobJoint::to_rapid() const {
    RapidRobJoint bin;
    std::memcpy(&bin.j1, &joints_[0], 4);
    std::memcpy(&bin.j2, &joints_[1], 4);
    std::memcpy(&bin.j3, &joints_[2], 4);
    std::memcpy(&bin.j4, &joints_[3], 4);
    std::memcpy(&bin.j5, &joints_[4], 4);
    std::memcpy(&bin.j6, &joints_[5], 4);
    return bin;
}
void RobJoint::set_joints(float j1, float j2, float j3, float j4, float j5, float j6) {
    joints_ = {j1, j2, j3, j4, j5, j6};
}

JointTargetBridge::JointTargetBridge(std::array<float, 6> joint_target, std::array<float, 6> ext_joints) : joints_(joint_target), ext_joints_(ext_joints) {}

JointTargetBridge::JointTargetBridge(const JointTargetBridge& other) : joints_(other.joints_), ext_joints_(other.ext_joints_) {}

// Default to 9E9 (not used in RAPID)
JointTargetBridge::~JointTargetBridge() {}

void JointTargetBridge::set_joints(float j1, float j2, float j3, float j4, float j5, float j6) {
    joints_ = {j1, j2, j3, j4, j5, j6};
}

RapidJointTarget JointTargetBridge::to_rapid() const {
    RapidJointTarget bin;
    std::memcpy(&bin.joints, &joints_[0], 24);
    std::memcpy(&bin.ext_joints, &ext_joints_[0], 24);
    return bin;
}

void JointTargetBridge::set_ext_joints(float ej1, float ej2, float ej3, float ej4, float ej5, float ej6) {
    ext_joints_ = {ej1, ej2, ej3, ej4, ej5, ej6};
}

std::string JointTargetBridge::to_string() const {
    std::stringstream ss;
    ss << "[JOINTTARGET] " << "Joints: " << "..." << " | Ext Joints: " << "...";
    return ss.str();
}

// --- RobTargetBridge ---
RobTargetBridge::RobTargetBridge(
    const RobPosition& position,
    const RobOrientation& orientation,
    const RobConfigData& config_data,
    const RobJoint& ext_joint
) : position_(position), orientation_(orientation), config_data_(config_data), ext_joint_(ext_joint) {}

RobTargetBridge::RobTargetBridge(const RobTargetBridge& other)
    : position_(other.position_), orientation_(other.orientation_),
      config_data_(other.config_data_), ext_joint_(other.ext_joint_) {}

RobTargetBridge::RobTargetBridge() : position_(), orientation_(), config_data_(), ext_joint_() {}
RobTargetBridge::~RobTargetBridge() {}

RapidRobTarget RobTargetBridge::to_rapid() const {
    RapidRobTarget bin;
    bin.position = position_.to_rapid();
    bin.orientation = orientation_.to_rapid();
    bin.config_data = config_data_.to_rapid();
    bin.ext_joint = ext_joint_.to_rapid();
    return bin;
}

void RobTargetBridge::set_target(const RobPosition& position, const RobOrientation& orientation,
                           const RobConfigData& config_data, const RobJoint& ext_joint) {
    position_ = position;
    orientation_ = orientation;
    config_data_ = config_data;
    ext_joint_ = ext_joint;
}

std::string RobTargetBridge::to_string() const {
    std::stringstream ss;
    ss << "[ROBTARGET] " << "Pos: " << "..." << " | Ori: " << "...";
    return ss.str();
}

} // namespace robert
