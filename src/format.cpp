#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;
using std::to_string;

string Format::ElapsedTime(long seconds) {
  long ss = seconds % 60;
  long totalMins = seconds / 60;

  long mm = totalMins % 60;
  long hh = totalMins / 60;

  return Pad(hh) + ":" + Pad(mm) + ":" + Pad(ss);
}

string Format::Pad(long number) {
  const string& basicString = to_string(number);

  if (number > 9) {
    return basicString;
  }

  return "0" + basicString;
}

string Format::PrecisionFloat(float number, int precision) {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << number;
  std::string s = stream.str();

  return s;
}