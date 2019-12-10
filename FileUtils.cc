#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "FileUtils.h"
#include "macro.h"
#include "Logger.h"

namespace sun
{

Dir::Dir(const char *pathname)
{
  dir_ = opendir(pathname);
}

Dir::Dir(DIR *dir)
{
  dir_ = dir;
}

Dir::~Dir()
{
  if (dir_)
    closedir(dir_);
}

Dir::operator bool() const
{
  return dir_;
}

std::vector<std::string> Dir::list(std::function<bool(const struct dirent *)> filter)
{
  std::vector<std::string> vec;
  struct dirent *entry;
  if (dir_) {
    while ((entry = readdir(dir_))) {
      if (!filter || filter(entry))
        vec.push_back(entry->d_name);
    }
  }
  return vec;
}

int Dir::mkdir(const char *pathname, mode_t mode)
{
  return mkdirat(dirfd(dir_), pathname, mode);
}

void Dir::mkdirp(const char *pathname, mode_t mode)
{
  char buf[2048];
  char *p, *s = buf;
  int fd, next;
  if (dir_) {
    strcpy(buf, pathname);
    // Only relative path is supported
    if (*buf == '/')
      ++s;
    fd = dup(dirfd(dir_));
    for (;(p = strchr(s, '/')); s = ++p) {
      *p = 0;
      mkdirat(fd, s, mode);
      next = openat(fd, s, O_DIRECTORY);
      if (next < 0) {
        close(fd);
        return;
      }
      close(fd);
      fd = next;
    }
    if (*s)
      mkdirat(fd, s, mode);
    close(fd);
  }
}

std::shared_ptr<Dir> Dir::cd(const char *pathname)
{
  int fd;
  DIR *dir = nullptr;
  if (dir_) {
    fd = openat(dirfd(dir_), pathname, O_DIRECTORY);
    if (fd != -1)
      dir = fdopendir(fd);
  }
  return std::make_shared<Dir>(dir);
}

std::string Dir::pwd()
{
  char pathname[128];
  char buf[2048];
  int size;
  if (dir_) {
    sprintf(pathname, "/proc/self/fd/%d", dirfd(dir_));
    size = readlink(pathname, buf, sizeof(buf));
    // readlink() does not append a null byte
    if (size < 0)
      return std::string{};
  }
  return std::string(buf, size);
}

File::File(const char *pathname)
  : path_(pathname), fd_(-1), size_(0)
{

}

File::~File()
{
  if (fd_ > 0)
    close(fd_);
}

File::operator bool() const
{
  return access(path_.c_str(), F_OK) != -1;
}

File::operator const char *() const
{
  return path_.c_str();
}

// Not work for /proc/[0-9]{1,}/cmdline
inline static int nread(int fd)
{
  int size = 0;
  ioctl(fd, FIONREAD, &size);
  return size;
}

void File::open(int flag, mode_t mode)
{
  // printf("flag:%x\n", flag);
  // Notice: O_RDWR != (O_RDONLY | O_WRONLY)
  fd_ = ::open(path_.c_str(), O_RDONLY | flag, mode);
  if (fd_ < 0) {
    Logger::getInstance().info("Open %s failed\n", path_.c_str());
    return;
  }
  size_ = nread(fd_);
}

#define PAGE_SIZE 4096
#define PAGE_CEIL(x) ((x + PAGE_SIZE - 1) & (~(PAGE_SIZE - 1)))

bool File::store(const void *buf, size_t count)
{
  void *p;
  bool ret = false;
  if (fd_ < 0)
    open(O_RDWR | O_CREAT);
  if (fd_ > 0) {
    ftruncate(fd_, count);
    p = mmap(NULL, count, PROT_WRITE, MAP_SHARED, fd_, 0);
    if (p == MAP_FAILED) {
      PERROR("(uid:%d)mmap %s(%zu) failed", (int)getuid(), path_.c_str(), count);
      // printf("seal:%x\n", fcntl(fd_, F_GET_SEALS));
      ftruncate(fd_, 0);
      goto end;
    }
    fast_copy(p, buf, count);
    fsync(fd_);
    munmap(p, count);
    ret = true;
  }
end:
  return false;
}

std::string File::load()
{
  std::string s;
  void *p;
  if (!bool(*this))
    goto end;
  if (fd_ < 0)
    open(0, 0444);
  if (fd_ > 0) {
    p = mmap(NULL, size_, PROT_READ, MAP_SHARED, fd_, 0);
    if (p == MAP_FAILED) {
      PERROR("(uid:%d)mmap %s(%zu) failed", (int)getuid(), path_.c_str(), size_);
      goto end;
    }
    s = std::string((char *)p, size_);
    munmap(p, size_);
  }
end:
  return s;
}

int File::pread(void *buf, size_t count, off_t offset)
{
  if (fd_ < 0)
    open(0, 0444);
  if (fd_ > 0)
    return ::pread(fd_, buf, count, offset);
  return 0;
}

int File::remove()
{
  return ::remove(path_.c_str());
}

size_t File::size()
{
  return size_;
}

}
