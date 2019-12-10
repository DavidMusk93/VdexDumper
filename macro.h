#pragma once

#include <stdint.h>

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define WHERE_FALSE while (0)

#define CTOR          __attribute__((constructor))
#define DTOR          __attribute__((destructor))
#define WEAK          __attribute__((weak))
#define WEAKREF(func) __attribute__((weakref(#func)))
#define UNUSED        __attribute__((unused))
#define DEPRECATED    __attribute__((deprecated))
#define ALIGNED(i)    __attribute__((aligned(i)))

#define bit_reset(state, i) \
  (state)[(i) / 8] &= ~(1 << ((i) % 8))
#define bit_set(state, i) \
  (state)[(i) / 8] |= (1 << ((i) % 8))
#define bit_get(state, i) \
  ((state)[(i) / 8] & (1 << ((i) % 8)))

#define STATE_MARK(state, flag) \
  state |= (flag)
#define STATE_UNMARK(state, flag) \
  state &= ~(flag)
#define STATE_TEST(state, flag) \
  (state & (flag))

#define nforeach(i, n) \
  for (int i = 0, __n = (int)(n); i < __n; ++i)

#define fast_copy(dst, src, size) do {   \
  int64_t *__d = (int64_t *)(dst);       \
  int64_t *__s = (int64_t *)(src);       \
  int __size = (int)(size);              \
  nforeach(i, __size >> 3) {             \
    __d[i] = __s[i];                     \
  }                                      \
  __d += (__size >> 3);                  \
  __s += (__size >> 3);                  \
  nforeach(j, __size & 7) {              \
    ((char *)__d)[j] = ((char *)__s)[j]; \
  }                                      \
} WHERE_FALSE

#define fast_reset(p, size) do {   \
  uint64_t *__p = (uint64_t *)(p); \
  int __size = (int)(size);        \
  nforeach(i, __size >> 3) {       \
    __p[i] = 0;                    \
  }                                \
  __p += (__size >> 3);            \
  nforeach(j, __size & 7) {        \
    ((uint8_t *)__p)[j] = 0;       \
  }                                \
} WHERE_FALSE

#define fast_cmp(p1, p2, size) ({      \
  uint64_t *__p1 = (uint64_t *)p1;     \
  uint64_t *__p2 = (uint64_t *)p2;     \
  int n, __size = (int)(size);         \
  nforeach(i, __size >> 3) {           \
    if ((n = __p1[i] - __p2[i])) {     \
      break;                           \
    }                                  \
  }                                    \
  if (!n && (__size & 7)) {            \
    __p1 += (__size >> 3);             \
    __p2 += (__size >> 3);             \
    nforeach(j, __size & 7) {          \
      if ((n = ((uint8_t *)__p1)[j]    \
            - ((uint8_t *)__p2)[j])) { \
        break;                         \
      }                                \
    }                                  \
  }                                    \
  n;                                   \
})

#define quick_hash(s) ({   \
  char *__s = (char *)(s); \
  int h = 0;               \
  char c;                  \
  while ((c = *__s++)) {   \
    h = (h << 5) - h + c;  \
  }                        \
  h;                       \
})

#define _basename(s) ({                 \
  char *__s = (char *)(s);              \
  char *slash;                          \
  (void)((slash = strrchr(__s, '/'))    \
      || (slash = strrchr(__s, '\\'))); \
  slash ? ++slash : s;                  \
})

#ifdef DEBUG
#include <unistd.h>
#include <fcntl.h>

#define DUMP_GAP (1 << 3)
#define dumpobj(b, n) do {                    \
  nforeach(i, n - 1) {                        \
    if ((i + 1) & (DUMP_GAP - 1)) {           \
      printf("%2.2x ", ((uint8_t *)(b))[i]);  \
    }                                         \
    else {                                    \
      printf("%2.2x\n", ((uint8_t *)(b))[i]); \
    }                                         \
  }                                           \
  printf("%2.2x\n", ((uint8_t *)(b))[n - 1]); \
} WHERE_FALSE

typedef struct fd_context_t {
  int stdout_bak;
  int fd_redirect;
} fd_context_t;

#define redirect_stdout(file) ({        \
  fd_context_t ctx;                     \
  ctx.fd_redirect = open(file,          \
      O_CREAT | O_WRONLY, 0644);        \
  ctx.stdout_bak = dup(STDOUT_FILENO);  \
  dup2(ctx.fd_redirect, STDOUT_FILENO); \
  ctx;                                  \
})

#define rewind_stdout(ctx) do {        \
  dup2(ctx.stdout_bak, STDOUT_FILENO); \
  close(ctx.stdout_bak);               \
  close(ctx.fd_redirect);              \
} WHERE_FALSE

#define dump2f(b, n, file) do {             \
  fd_context_t ctx = redirect_stdout(file); \
  dumpobj(b, n);                            \
  rewind_stdout(ctx);                       \
} WHERE_FALSE
#else
#define dumpobj(b, n)
#define redirect_stdout(file)
#define rewind_stdout(ctx)
#define dump2f(b, n, file)
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) (-MIN(-(a), -(b)))
#define ARR_SIZE(a) (int)(sizeof(a) / sizeof(*a))

#define gettime(p) \
  clock_gettime(CLOCK_REALTIME, p)

#ifdef COUNT_TIME
#include <time.h>

typedef struct tr_context_t {
  int line;
  struct timespec ts;
} tr_context_t;

static tr_context_t _tr_ctx;

#define tr_head() do {     \
  _tr_ctx.line = __LINE__; \
  gettime(&_tr_ctx.ts);    \
} WHERE_FALSE

#define tr_tail() do {              \
  tr_context_t ctx = _tr_ctx;       \
  tr_head();                        \
  float diff = (float)(             \
      _tr_ctx.ts.tv_sec -           \
      ctx.ts.tv_sec) * 1000. + (    \
        _tr_ctx.ts.tv_nsec -        \
        ctx.ts.tv_nsec) / 1000000.; \
  fprintf(stdout,                   \
      "[%d-%d] %.3f (ms)\n",        \
      ctx.line, _tr_ctx.line,       \
      diff);                        \
} WHERE_FALSE
#else
#define tr_head()
#define tr_tail()
#endif
