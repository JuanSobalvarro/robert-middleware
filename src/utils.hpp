#pragma once

#include <string>


namespace robert::utils
{

std::string trim_copy(const std::string& text);

std::string format_double(double value, int precision = 4);

}
