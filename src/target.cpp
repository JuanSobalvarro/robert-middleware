#include "target.hpp"
#include <sstream>
#include <iomanip>

// --- RobPosition ---
robert::RobPosition::RobPosition(double x, double y, double z) : position_{x, y, z} {}
robert::RobPosition::RobPosition(const RobPosition& other) : position_(other.position_) {}
robert::RobPosition::RobPosition() : position_{0.0, 0.0, 0.0} {}
robert::RobPosition::~RobPosition() {}

double robert::RobPosition::x() const { return position_[0]; }
double robert::RobPosition::y() const { return position_[1]; }
double robert::RobPosition::z() const { return position_[2]; }

void robert::RobPosition::set_position(double x, double y, double z) {
    position_ = {x, y, z};
}

std::string robert::RobPosition::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[" << position_[0] << "," << position_[1] << "," << position_[2] << "]";
    return oss.str();
}

// --- RobOrientation ---
robert::RobOrientation::RobOrientation(double q1, double q2, double q3, double q4) 
    : qrotation_{q1, q2, q3, q4} 
{}

robert::RobOrientation::RobOrientation(const RobOrientation& other) 
    : qrotation_(other.qrotation_) 
{}

robert::RobOrientation::RobOrientation() 
    : qrotation_{0.0, 0.0, 0.0, 1.0} 
{}

robert::RobOrientation::~RobOrientation() 
{}

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
    qrotation_ = {q1, q2, q3, q4};
}

std::string robert::RobOrientation::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6); // Quaternions need high precision
    oss << "[" << qrotation_[0] << "," << qrotation_[1] << "," << qrotation_[2] << "," << qrotation_[3] << "]";
    return oss.str();
}

// --- RobConfigData ---
robert::RobConfigData::RobConfigData(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx) 
    : cf_{cf1, cf4, cf6}, cfx_(cfx) {}
robert::RobConfigData::RobConfigData(const RobConfigData& other) : cf_(other.cf_), cfx_(other.cfx_) {}
robert::RobConfigData::RobConfigData() : cf_{0, 0, 0}, cfx_(0) {}
robert::RobConfigData::~RobConfigData() {}

void robert::RobConfigData::set_config_data(signed int cf1, signed int cf4, signed int cf6, unsigned int cfx) {
    if (cfx > 7) throw std::out_of_range("cfx must be between 0 and 7");
    cf_ = {cf1, cf4, cf6};
    cfx_ = cfx;
}

std::string robert::RobConfigData::to_string() const {
    return "[" + std::to_string(cf_[0]) + "," + std::to_string(cf_[1]) + "," + 
                 std::to_string(cf_[2]) + "," + std::to_string(cfx_) + "]";
}

// --- RobTarget ---
robert::RobTarget::RobTarget(const RobPosition& position, const RobOrientation& orientation, const RobConfigData& config_data)
    : position_(position), orientation_(orientation), config_data_(config_data) {}

robert::RobTarget::RobTarget() : position_(), orientation_(), config_data_() {}

robert::RobTarget::RobTarget(const RobTarget& other)
    : position_(other.position_), orientation_(other.orientation_), config_data_(other.config_data_) {}

robert::RobTarget::~RobTarget() {}

std::string robert::RobTarget::to_string() const {
    // We MUST include the external axes [9e9, 9e9, 9e9, 9e9, 9e9, 9e9] 
    // for a robtarget variable to be valid in RAPID.
    return position_.to_string() + 
                 orientation_.to_string() + 
                 config_data_.to_string() + 
                 "[9e9,9e9,9e9,9e9,9e9,9e9]";
}