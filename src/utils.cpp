#include "utils.hpp"

#include <iomanip>
#include <algorithm>
#include <sstream>

namespace robert::utils {

std::string trim_copy(const std::string& text) {
    std::string value = text;
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char c) {
        return !std::isspace(c);
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char c) {
        return !std::isspace(c);
    }).base(), value.end());
    return value;
}

std::string format_double(double value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

} // namespace robert::utils
