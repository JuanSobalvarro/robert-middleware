#include "target.hpp"
#include <cstring>
#include <sstream>
#include <iomanip>

namespace robert
{

// --- RobPosition ---
RobPosition::RobPosition(float x, float y, float z) : position_{x, y, z} {}
RobPosition::RobPosition(const RobPosition& other) : position_(other.position_) {}
RobPosition::RobPosition() : position_{0.0f, 0.0f, 0.0f} {}
RobPosition::~RobPosition() {}

void RobPosition::set_position(float x, float y, float z) {
    position_ = {x, y, z};
}

BinRobPosition RobPosition::to_bin() const {
    BinRobPosition bin;
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

BinRobOrientation RobOrientation::to_bin() const {
    BinRobOrientation bin;
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

BinRobConfigData RobConfigData::to_bin() const {
    BinRobConfigData bin;
    bin.cf1 = config_data_[0];
    bin.cf4 = config_data_[1];
    bin.cf6 = config_data_[2];
    bin.cfx = static_cast<uint32_t>(config_data_[3]);
    return bin;
}

// --- RobExtJoint ---
RobJoint::RobJoint(float j1, float j2, float j3, float j4, float j5, float j6) 
    : ext_joint_{j1, j2, j3, j4, j5, j6} {}
RobJoint::RobJoint(const RobJoint& other) : ext_joint_(other.ext_joint_) {}
// Default to 9E9 (not used in RAPID)
RobJoint::RobJoint() : ext_joint_{9e9f, 9e9f, 9e9f, 9e9f, 9e9f, 9e9f} {}
RobJoint::~RobJoint() {}

void RobJoint::set_ext_joint(float j1, float j2, float j3, float j4, float j5, float j6) {
    ext_joint_ = {j1, j2, j3, j4, j5, j6};
}

BinRobJoint RobJoint::to_bin() const {
    BinRobJoint bin;
    std::memcpy(&bin.j1, &ext_joint_[0], 4);
    std::memcpy(&bin.j2, &ext_joint_[1], 4);
    std::memcpy(&bin.j3, &ext_joint_[2], 4);
    std::memcpy(&bin.j4, &ext_joint_[3], 4);
    std::memcpy(&bin.j5, &ext_joint_[4], 4);
    std::memcpy(&bin.j6, &ext_joint_[5], 4);
    return bin;
}

// --- RobTarget ---
RobTarget::RobTarget(const RobPosition& position, const RobOrientation& orientation, 
                     const RobConfigData& config_data, const RobJoint& ext_joint)
    : position_(position), orientation_(orientation), config_data_(config_data), ext_joint_(ext_joint) {}

RobTarget::RobTarget(const RobTarget& other) 
    : position_(other.position_), orientation_(other.orientation_), 
      config_data_(other.config_data_), ext_joint_(other.ext_joint_) {}

RobTarget::RobTarget() : position_(), orientation_(), config_data_(), ext_joint_() {}
RobTarget::~RobTarget() {}

BinRobTarget RobTarget::to_bin() const {
    BinRobTarget bin;
    bin.position = position_.to_bin();
    bin.orientation = orientation_.to_bin();
    bin.config_data = config_data_.to_bin();
    bin.ext_joint = ext_joint_.to_bin();
    return bin;
}

void RobTarget::set_target(const RobPosition& position, const RobOrientation& orientation, 
                           const RobConfigData& config_data, const RobJoint& ext_joint) {
    position_ = position;
    orientation_ = orientation;
    config_data_ = config_data;
    ext_joint_ = ext_joint;
}

std::string RobTarget::to_string() const {
    std::stringstream ss;
    ss << "[ROBTARGET] " << "Pos: " << "..." << " | Ori: " << "..."; 
    return ss.str();
}

} // namespace robert