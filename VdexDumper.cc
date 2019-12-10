#include <string.h>
#include <sys/ptrace.h>
#include <algorithm>
// #include <fstream>
#include "VdexDumper.h"
#include "FileUtils.h"
#include "Logger.h"
#include "VdexHeader.h"
#include "StringUitls.h"

namespace sun
{

#define attachProcess(pid) ptrace(PTRACE_ATTACH, pid, nullptr, nullptr)
#define detachProcess(pid) ptrace(PTRACE_DETACH, pid, nullptr, nullptr);

VdexDumper::VdexDumper(const std::string& package)
  : tid_(-1), mem_fd_(-1)
{
  char path[128];
  pid_ = ProcHelper::findProcess(package);
  tid_ = (int)ProcHelper::LastTaskId(pid_);
  if (tid_) {
    if (attachProcess(tid_) == 0) {
      sprintf(path, "/proc/%d/mem", tid_);
      mem_fd_ = ::open(path, O_RDONLY);
      if (mem_fd_ < 0) {
        PERROR("Open %s failed", path);
        detachProcess(tid_);
        return;
      }
    }
    PERROR("ptrace");
  }
}

VdexDumper::~VdexDumper()
{
  if (mem_fd_ > 0)
    detachProcess(tid_);
  bzero(this, sizeof(VdexDumper));
}

std::vector<MemRegion> VdexDumper::findVdex()
{
  std::vector<MemRegion> vec;
  FILE *fp;
  char path[1024];
  // char header[8];
  VdexHeader header;
  char *endptr;
  char *buf = nullptr;
  size_t count = 0;
  if (mem_fd_ > 0) {
    sprintf(path, "/proc/%d/maps", pid_);
    fp = fopen(path, "r");
    if (!fp) {
      PERROR("Open %s failed", path);
      goto end;
    }
    while (getline(&buf, &count, fp) != -1) {
      MemRegion region;
      region.start = strtoull(buf, &endptr, 16);
      region.end = strtoull(++endptr, nullptr, 16);
      if (region.end - region.start < VDEX_MIN_LENGTH)
        continue;
      pread64(mem_fd_, &header, sizeof(header), region.start);
      if (strcmp(header.GetMagic(), kVdexMagicAndVersion) != 0)
        continue;
      // *header = 0;
      // pread(mem_fd_, header, sizeof(header), region.start);
      // if (*header && strcmp(header, kVdexMagicAndVersion) != 0)
      //   continue;

      *path = 0;
      sscanf(buf, "%*[^-]-%*[^ ]%*s%*s%*s%*s%s", path);
      if (!*path)
        continue;
      // if (!*path ||
      //     strncmp(path, VDEX_PATH_PREFIX, sizeof(VDEX_PATH_PREFIX) - 1) != 0)
      //   continue;
      region.name = path;
      vec.emplace_back(region);
    }
  }
end:
  if (buf)
    free(buf);
  return vec;
}

int VdexDumper::dumpVdex()
{
  void *buf = nullptr;
  size_t count = 0, gap;
  char path[256];
  auto vec = findVdex();
  if (!vec.empty()) {
    Dir secure(SECURE_DIR);
    if (bool(secure)) {
      for (const auto& region: vec) {
        auto s1 = StringUtils::dirname(region.name);
        // Logger::getInstance().info("(%s)mkdir:%s\n", secure.pwd().c_str(), s1.c_str());
        secure.mkdirp(s1.c_str());
        sprintf(path, SECURE_DIR "%s", region.name.c_str());
        File f(path);
        gap = region.end - region.start;
        if (gap > count) {
          count = gap;
          buf = realloc(buf, count);
        }
        pread64(mem_fd_, buf, gap, region.start);
        // lseek64(mem_fd_, region.start, SEEK_SET);
        // read(mem_fd_, buf, gap);
        Logger::getInstance().info("Save %s\n", (const char *)f);
        f.store(buf, gap);
      }
      if (buf)
        free(buf);
    }
  }
  return vec.size();
}

uint32_t ProcHelper::LastTaskId(uint32_t pid)
{
  uint32_t tid = 0;
  char path[128];
  sprintf(path, "/proc/%u/task", pid);
  Dir dir(path);
  if (bool(dir)) {
    auto vec = dir.list();
    if (vec.size() < 3)
      goto end;
    std::sort(vec.begin(), vec.end());
    tid = std::stoul(vec.back());
  }
end:
  return tid;
}

uint32_t ProcHelper::findProcess(const std::string& cmdline)
{
  auto pidFilter = [](const dirent *entry) {
    return atoi(entry->d_name) > 1;
  };
  Dir dir("/proc");
  char path[128];
  char buf[256];
  if (bool(dir)) {
    auto pids = dir.list(pidFilter);
    for (const auto& pid: pids) {
      sprintf(path, "/proc/%s/cmdline", pid.c_str());
      File f(path);
      f.pread(buf, sizeof(buf));
      // printf("buf:(%s)%.*s\n", pid.c_str(), (int)sizeof(buf), buf);
      if (strcmp(buf, cmdline.c_str()) == 0)
        return std::stoul(pid);
      // auto s = f.load();
      // debug("walk %s:%s:%s", pid.c_str(), s.c_str(), buf);
      // if (s == cmdline)
      //   return std::stoul(s);
    }
  }
  return 0;
}

}
