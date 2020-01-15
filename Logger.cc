#include "Logger.h"

namespace sun
{

Logger Logger::instance;
Logger &Logger::getInstance() { return Logger::instance; }

void Logger::info(const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}

}
