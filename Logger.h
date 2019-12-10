#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "noncopyable.h"

namespace sun
{

class Logger: public noncopyable
{
 public:
  void info(const char *fmt, ...);
  static Logger instance;
  static Logger &getInstance();
};

}

#define PERROR(fmt, ...) fprintf(stderr, fmt ":%s\n", ##__VA_ARGS__, strerror(errno))
