#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string ElapsedTime(long times);
std::string Pad(long number);
std::string PrecisionFloat(float number, int precision);
};  // namespace Format

#endif