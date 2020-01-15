#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#define SECURE_DIR "/data/local/tmp"
#define VDEX_PREFIX "dumped_"
#define VDEX_MIN_LENGTH 10240
#define VDEX_PATH_PREFIX "/data"

namespace sun {

struct MemRegion
{
  uint64_t start;
  uint64_t end;
  std::string name;
};

class VdexDumper
{
 public:
  VdexDumper(const std::string& package);
  ~VdexDumper();
  std::vector<MemRegion> findVdex();
  int dumpVdex();

 private:
  int pid_;
  int mem_fd_;
};

class ProcHelper
{
  public:
    static uint32_t LastTaskId(uint32_t pid);
    static uint32_t findProcess(const std::string& cmdline);
};

}
