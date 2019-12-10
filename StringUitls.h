#pragma once

#include <string>

namespace sun
{

class StringUtils
{
 public:
  static std::string dirname(const std::string& str);
  static std::string basename(const std::string& str);
};

}
