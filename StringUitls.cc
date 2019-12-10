#include "StringUitls.h"

namespace sun
{

static const char kSlash = '/';

std::string StringUtils::dirname(const std::string& str)
{
  auto pos = str.find_last_of(kSlash);
  if (pos == std::string::npos)
    return std::string(".");
  return str.substr(0, pos);
}

std::string StringUtils::basename(const std::string& str)
{
  auto pos = str.find_last_of(kSlash);
  if (pos == std::string::npos)
    return str;
  return str.substr(pos + 1);
}

}
