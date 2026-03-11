#pragma once

#include <array>
#include <string>
#include <stdexcept>
#include <cstdint>

namespace robert
{

#pragma pack(push, 1) // forces the compiler to not add padding bytes (1 byte aligning)

struct BinRobPosition {
    uint32_t x; // this means float
    uint32_t y;
    uint32_t z;
};

struct BinRobOrientation {
    uint32_t q1;
    uint32_t q2;
    uint32_t q3;
    uint32_t q4;
};

struct BinRobConfigData {
    int32_t cf1; // conf are signed ints
    int32_t cf4;
    int32_t cf6;
    uint32_t cfx; // cfx is unsigned int
};

struct BinRobJoint {
    uint32_t j1;
    uint32_t j2;
    uint32_t j3;
    uint32_t j4;
    uint32_t j5;
    uint32_t j6;
};

struct BinRobTarget {
    BinRobPosition position; // 12 bytes
    BinRobOrientation orientation; // 16 bytes
    BinRobConfigData config_data; // 16 bytes
    BinRobJoint ext_joint; // 24 bytes
    // total: 68 bytes
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
    RobJoint();
    ~RobJoint();

    BinRobJoint to_bin() const;

    void set_ext_joint(float j1, float j2, float j3, float j4, float j5, float j6);
private:
    std::array<float, 6> ext_joint_;
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