#pragma once

#include <array>
#include <string>
#include <cstdint>

namespace robert::data
{

#pragma pack(push, 1) // forces the compiler to not add padding bytes (1 byte aligning)

/**
 * @struct RapidPosition
 * @brief Represents the position of the robot in 3D space. Each coordinate is stored as a 32-bit unsigned integer,
 * which corresponds to a float value when interpreted correctly. This allows for efficient binary packing while maintaining
 * the necessary precision for robotic movements.
 * Size: 12 bytes (3 coordinates x 4 bytes each)
 */
struct RapidPosition {
    uint32_t x; // this means float
    uint32_t y;
    uint32_t z;
};

/**
 * @struct RapidOrientation
 * @brief Represents the orientation of the robot using quaternions. Each component of the quaternion is stored
 * as a 32-bit unsigned integer, which corresponds to a float value when interpreted correctly. This allows for
 * efficient binary packing while maintaining the necessary precision for robotic movements.
 * Size: 16 bytes (4 components x 4 bytes each)
 */
struct RapidOrientation {
    uint32_t q1;
    uint32_t q2;
    uint32_t q3;
    uint32_t q4;
};

/**
 * @struct RapidConfData
 * @brief Represents the configuration data of the robot, including the configuration flags (cf1, cf4, cf6) and the cfx value.
 * The configuration flags are stored as signed 32-bit integers, while the cfx value is stored as an unsigned 32-bit integer.
 * Size: 16 bytes (3 signed ints x 4 bytes each + 1 unsigned int x 4 bytes)
 * Note: The configuration flags (cf1, cf4, cf6) are typically used to represent the robot's configuration state, while cfx is often used for additional configuration information.
 */
struct RapidConfData {
    int32_t cf1; // conf are signed ints
    int32_t cf4;
    int32_t cf6;
    uint32_t cfx; // cfx is unsigned int, why int32 even though cfx is between 0 and 7, because of 4 byte alignment
};

/**
 * @struct RapidRobJoint
 * @brief Represents the joint angles of the robot or the linear mm for external axes. Each joint angle is stored as a 32-bit unsigned integer, which corresponds to a float value when interpreted correctly. This allows for efficient binary packing while maintaining the necessary precision for robotic movements.
 */
struct RapidRobJoint {
    uint32_t j1;
    uint32_t j2;
    uint32_t j3;
    uint32_t j4;
    uint32_t j5;
    uint32_t j6;
};

/**
 * @struct RapidRobTarget
 * @brief Represents a complete target for the robot, including position, orientation, configuration data, and external joint angles.
 * The position is represented by a RapidPosition struct, the orientation by a RapidOrientation struct, the configuration data by a RapidConfData struct, and the external joint angles by a RapidRobJoint struct. This structure allows for efficient binary packing of all necessary target information for robotic commands.
 *
 * Size: 65 bytes (12 for position + 16 for orientation + 13 for config data + 24 for external joint_target)
 *
 * Note: The RapidRobTarget structure is designed to encapsulate all the necessary information for a robot target in a compact binary format. The position and orientation define the robot's pose in space, while the configuration data provides additional context about the robot's state. The external joint angles can be used for commands that require specific joint configurations, such as MoveAbsJ.
 */
struct RapidRobTarget {
    RapidPosition position; // 12 bytes
    RapidOrientation orientation; // 16 bytes
    RapidConfData config_data; // 13 bytes
    RapidRobJoint ext_joint; // 24 bytes
};

struct RapidJointTarget {
    RapidRobJoint joints; // 24 bytes
    RapidRobJoint ext_joints; // 24 bytes
};

#pragma pack(pop) // end of packing

class RobPosition {

public:
    RobPosition(float x, float y, float z);
    RobPosition(const RobPosition& other);
    RobPosition();
    ~RobPosition();

    RapidPosition to_rapid() const;

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

    RapidOrientation to_rapid() const;

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

    RapidConfData to_rapid() const;

    void set_config_data(int cf1, int cf4, int cf6, unsigned int cfx);
private:
    std::array<int, 4> config_data_; // we can store cfx as int and convert it to unsigned int when needed, since cfx is always positive
};

class RobJoint {
public:
    RobJoint(float j1, float j2, float j3, float j4, float j5, float j6);
    RobJoint(const RobJoint& other);
    RobJoint();
    ~RobJoint();

    RapidRobJoint to_rapid() const;

    void set_joints(float j1, float j2, float j3, float j4, float j5, float j6);

private:
    std::array<float, 6> joints_;
};

class JointTargetBridge {
public:
    JointTargetBridge(std::array<float, 6> joint_target = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, std::array<float, 6> ext_joints = {9e9f, 9e9f, 9e9f, 9e9f, 9e9f, 9e9f});
    JointTargetBridge(const JointTargetBridge& other);
    ~JointTargetBridge();

    RapidJointTarget to_rapid() const;

    std::string to_string() const;
    void set_joints(float j1, float j2, float j3, float j4, float j5, float j6);
    void set_ext_joints(float ej1, float ej2, float ej3, float ej4, float ej5, float ej6);
private:
    std::array<float, 6> joints_;
    std::array<float, 6> ext_joints_;

};

class RobTargetBridge {
public:
    RobTargetBridge(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data, const RobJoint& ext_joint);
    RobTargetBridge(const RobTargetBridge& other);
    RobTargetBridge();
    ~RobTargetBridge();

    RapidRobTarget to_rapid() const;

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
