#include <stdio.h>
#include <string.h>
#include "FileUtils.h"
#include "VdexDumper.h"

#define TEST_DUMPER 1

int main(int argc, char *argv[])
{
#if TEST_DIR
  std::vector<const char *> names{"test1", "test2", "test3"};
  auto handler = std::make_shared<sun::Dir>(".");
  handler->mkdirp("aaa/bbb/ccc");
  if (bool(handler.get())) {
    for (const auto& name: names) {
      handler->mkdir(name, 0777);
      handler = handler->cd(name);
    }
  }
  auto vec = handler->list(nullptr);
  for (const auto& f: vec)
    puts(f.c_str());
  printf("pwd:%s\n", handler->pwd().c_str());
  (void)getchar();
#endif

#if TEST_FILE
  const char *data = "This is a test!";
  sun::File file("test_store");
  file.store(data, strlen(data));

  sun::File f1("./tobedeleted");
  auto s = f1.load();
  printf("content:'%s'\n", s.c_str());
  f1.remove();
#endif

#if TEST_DUMPER
  // auto pid = sun::ProcHelper::findProcess("/usr/sbin/sshd");
  // printf("sshd:%u\n", pid);
  // auto tid = sun::ProcHelper::LastTaskId(pid);
  // printf("sshd-tid:%u\n", tid);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s package-name\n", argv[0]);
    return 1;
  }
  auto dumper = sun::VdexDumper(argv[1]);
  dumper.dumpVdex();
#endif
  return 0;
}
