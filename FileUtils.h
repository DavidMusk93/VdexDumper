#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "noncopyable.h"

namespace sun
{

class Dir: public noncopyable
{
 public:
  Dir(const char *pathname);
  Dir(DIR *dir);
  ~Dir();
  operator bool() const;
  std::vector<std::string> list(std::function<bool(const struct dirent *)> filter = nullptr);
  int mkdir(const char *pathname, mode_t mode = 0777);
  void mkdirp(const char *pathname, mode_t mode = 0777);
  std::shared_ptr<Dir> cd(const char *pathname);
  std::string pwd();

 private:
  DIR *dir_;
};

class File: public noncopyable
{
 public:
  File(const char *pathname);
  ~File();
  operator bool() const;
  operator const char *() const;
  void open(int flag = 0, mode_t mode = 0666);
  bool store(const void *buf, size_t count);
  std::string load();
  int pread(void *buf, size_t count, off_t offset = 0);
  int remove();
  size_t size();

 private:
  std::string path_;
  int fd_;
  size_t size_;
};

}


