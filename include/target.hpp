#pragma once

#include <array>
#include <string>
#include <stdexcept>

namespace robert
{

class RobPosition
{
public:
    RobPosition(double x, double y, double z);
    RobPosition(const RobPosition& other);
    RobPosition();
    ~RobPosition();

    double x() const;
    double y() const;
    double z() const;

    void set_position(double x, double y, double z);
    std::string to_string() const;
private:
    std::array<double, 3> position_;
};

class RobOrientation
{
public:
    RobOrientation(double q1, double q2, double q3, double q4);
    RobOrientation(const RobOrientation& other);
    RobOrientation();
    ~RobOrientation();

    double q1() const;
    double q2() const;
    double q3() const;
    double q4() const;

    void set_orientation(double q1, double q2, double q3, double q4);
    std::string to_string() const;
private:
    std::array<double, 4> qrotation_;
};

class RobConfigData
{
public:
    RobConfigData(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx);
    RobConfigData(const RobConfigData& other);
    RobConfigData();
    ~RobConfigData();

    signed int cf1() const;
    signed int cf4() const;
    signed int cf6() const;
    unsigned int cfx() const;

    void set_config_data(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx);
    std::string to_string() const;
private:
    std::array<signed int, 3> cf_;
    unsigned int cfx_; 
};

class RobTarget
{
public:
    RobTarget(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data);
    RobTarget(std::array<double, 3> position, std::array<double, 4> orientation, std::array<signed int, 3> config_data, unsigned int cfx);
    RobTarget(const RobTarget& other);
    RobTarget();
    ~RobTarget();

    std::string to_string() const;
private:
    RobPosition position_;
    RobOrientation orientation_;
    RobConfigData config_data_;
};

} // namespace robert