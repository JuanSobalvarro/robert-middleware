#include "target.hpp"

robert::RobPosition::RobPosition(double x, double y, double z)
    : position_{x, y, z}
{
}

robert::RobPosition::RobPosition(const RobPosition& other)
    : position_(other.position_)
{
}

robert::RobPosition::RobPosition()
    : position_{0.0, 0.0, 0.0}
{
}

robert::RobPosition::~RobPosition()
{
}

double robert::RobPosition::x() const
{
    return position_[0];
}

double robert::RobPosition::y() const
{
    return position_[1];
}

double robert::RobPosition::z() const
{
    return position_[2];
}

void robert::RobPosition::set_position(double x, double y, double z)
{
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
}

robert::RobOrientation::RobOrientation(double q1, double q2, double q3, double q4)
    : qrotation_{q1, q2, q3, q4}
{
}

robert::RobOrientation::RobOrientation(const RobOrientation& other)
    : qrotation_(other.qrotation_)
{
}

robert::RobOrientation::RobOrientation()
    : qrotation_{0.0, 0.0, 0.0, 1.0} // Identity quaternion
{
}

robert::RobOrientation::~RobOrientation()
{
}

double robert::RobOrientation::q1() const
{
    return qrotation_[0];
}

double robert::RobOrientation::q2() const
{
    return qrotation_[1];
}

double robert::RobOrientation::q3() const
{
    return qrotation_[2];
}

double robert::RobOrientation::q4() const
{
    return qrotation_[3];
}

void robert::RobOrientation::set_orientation(double q1, double q2, double q3, double q4)
{
    qrotation_[0] = q1;
    qrotation_[1] = q2;
    qrotation_[2] = q3;
    qrotation_[3] = q4;
}

robert::RobConfigData::RobConfigData(const RobConfigData& other)
    : cf_(other.cf_), cfx_(other.cfx_)
{
}

robert::RobConfigData::RobConfigData(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx)
    : cf_{cf1, cf4, cf6}, cfx_(cfx)
{
}

robert::RobConfigData::RobConfigData()
    : cf_{0, 0, 0}, cfx_(0)
{
}

robert::RobConfigData::~RobConfigData()
{
}

signed int robert::RobConfigData::cf1() const
{
    return cf_[0];
}

signed int robert::RobConfigData::cf4() const
{
    return cf_[1];
}

signed int robert::RobConfigData::cf6() const
{
    return cf_[2];
}

unsigned int robert::RobConfigData::cfx() const
{
    return cfx_;
}

void robert::RobConfigData::set_config_data(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx)
{
    cf_[0] = cf1;
    cf_[1] = cf4;
    cf_[2] = cf6;
    cfx_ = cfx;
}

robert::RobTarget::RobTarget(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data)
    : position_(position), orientation_(orientation), config_data_(config_data)
{
}

robert::RobTarget::RobTarget(std::array<double, 3> position, std::array<double, 4> orientation, std::array<signed int, 3> config_data, unsigned int cfx)
{
}

robert::RobTarget::~RobTarget()
{
}
