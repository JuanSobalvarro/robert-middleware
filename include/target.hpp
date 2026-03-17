#pragma once

#include <array>
#include <string>
#include <stdexcept>
#include <cstdint>

namespace robert
{

#pragma pack(push, 1) // forces the compiler to not add padding bytes (1 byte aligning)

/**
 * @struct BinRobPosition
 * @brief Represents the position of the robot in 3D space. Each coordinate is stored as a 32-bit unsigned integer, 
 * which corresponds to a float value when interpreted correctly. This allows for efficient binary packing while maintaining 
 * the necessary precision for robotic movements.
 * Size: 12 bytes (3 coordinates x 4 bytes each)
 */
struct BinRobPosition {
    uint32_t x; // this means float
    uint32_t y;
    uint32_t z;
};

/**
 * @struct BinRobOrientation
 * @brief Represents the orientation of the robot using quaternions. Each component of the quaternion is stored
 * as a 32-bit unsigned integer, which corresponds to a float value when interpreted correctly. This allows for 
 * efficient binary packing while maintaining the necessary precision for robotic movements.
 * Size: 16 bytes (4 components x 4 bytes each)
 */
struct BinRobOrientation {
    uint32_t q1;
    uint32_t q2;
    uint32_t q3;
    uint32_t q4;
};

/**
 * @struct BinRobConfigData
 * @brief Represents the configuration data of the robot, including the configuration flags (cf1, cf4, cf6) and the cfx value.
 * The configuration flags are stored as signed 32-bit integers, while the cfx value is stored as an unsigned 32-bit integer.
 * Size: 16 bytes (3 signed ints x 4 bytes each + 1 unsigned int x 4 bytes)
 * Note: The configuration flags (cf1, cf4, cf6) are typically used to represent the robot's configuration state, while cfx is often used for additional configuration information.
 */
struct BinRobConfigData {
    int32_t cf1; // conf are signed ints
    int32_t cf4;
    int32_t cf6;
    uint32_t cfx; // cfx is unsigned int, why int32 even though cfx is between 0 and 7, because of 4 byte alignment
};

/**
 * @struct BinRobJoint
 * @brief Represents the joint angles of the robot. Each joint angle is stored as a 32-bit unsigned integer, which corresponds 
 * to a float value when interpreted correctly. This allows for efficient binary packing while maintaining the necessary 
 * precision for robotic movements.
 * Size: 24 bytes (6 joints x 4 bytes each)
 * Note: The joint angles are typically used for MoveAbsJ commands, where the robot's movement is defined in terms of joint 
 * angles rather than Cartesian coordinates. The ext_joint field in the BinRobTarget can also be used to specify external joint 
 * angles for certain commands.
 */
struct BinRobJoint {
    uint32_t j1;
    uint32_t j2;
    uint32_t j3;
    uint32_t j4;
    uint32_t j5;
    uint32_t j6;
};

/**
 * @struct BinRobTarget
 * @brief Represents a complete target for the robot, including position, orientation, configuration data, and external joint angles.
 * The position is represented by a BinRobPosition struct, the orientation by a BinRobOrientation struct, the configuration data by a BinRobConfigData struct, and the external joint angles by a BinRobJoint struct. This structure allows for efficient binary packing of all necessary target information for robotic commands.
 * 
 * Size: 65 bytes (12 for position + 16 for orientation + 13 for config data + 24 for external joints)
 * 
 * Note: The BinRobTarget structure is designed to encapsulate all the necessary information for a robot target in a compact binary format. The position and orientation define the robot's pose in space, while the configuration data provides additional context about the robot's state. The external joint angles can be used for commands that require specific joint configurations, such as MoveAbsJ.
 */
struct BinRobTarget {
    BinRobPosition position; // 12 bytes
    BinRobOrientation orientation; // 16 bytes
    BinRobConfigData config_data; // 13 bytes
    BinRobJoint ext_joint; // 24 bytes
};

struct BinJointTarget {
    BinRobJoint joints; // 24 bytes
    BinRobJoint ext_joints; // 24 bytes
};

#pragma pack(pop) // end of packing

class RobPosition {

public:
    RobPosition(float x, float y, float z);
    RobPosition(const RobPosition& other);
    RobPosition();
    ~RobPosition();

    BinRobPosition to_bin() const;

    void set_position(float x, float y, float z);
private:
    std::array<float, 3> position_;
};

class RobOrientation {

public:
    RobOrientation(float q1, float q2, float q3, float q4);
    RobOrientation(const RobOrientation& other);
    RobOrientation();
    ~RobOrientation();

    BinRobOrientation to_bin() const;

    void set_orientation(float q1, float q2, float q3, float q4);
private:
    std::array<float, 4> orientation_;
};

class RobConfigData {
public:
    RobConfigData(int cf1, int cf4, int cf6, unsigned int cfx);
    RobConfigData(const RobConfigData& other);
    RobConfigData();
    ~RobConfigData();

    BinRobConfigData to_bin() const;

    void set_config_data(int cf1, int cf4, int cf6, unsigned int cfx);
private:
    std::array<int, 4> config_data_; // we can store cfx as int and convert it to unsigned int when needed, since cfx is always positive
};

class RobJoint {
public:
    RobJoint(float j1 = 9e9f, float j2 = 9e9f, float j3 = 9e9f, float j4 = 9e9f, float j5 = 9e9f, float j6 = 9e9f);
    RobJoint(const RobJoint& other);
    ~RobJoint();

    BinRobJoint to_bin() const;

    void set_joints(float j1, float j2, float j3, float j4, float j5, float j6);
private:
    std::array<float, 6> joints_;
};

class RobTarget {
public:
    RobTarget(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data, const RobJoint& ext_joint);
    RobTarget(const RobTarget& other);
    RobTarget();
    ~RobTarget();

    BinRobTarget to_bin() const;

    std::string to_string() const;
    void set_target(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data, const RobJoint& ext_joint);
private:
    RobPosition position_;
    RobOrientation orientation_;
    RobConfigData config_data_;
    RobJoint ext_joint_;
};

//
} // namespace robert