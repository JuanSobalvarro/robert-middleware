#pragma once

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>


namespace robert
{

std::string trim_copy(const std::string& text);

std::string format_double(double value, int precision = 4);

}