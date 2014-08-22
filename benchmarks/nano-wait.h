
#include <unistd.h>
#include <stdint.h>
#include <time.h>


static inline void nano_wait (uint32_t s, uint32_t ns) {
  struct timespec ts;
  ts.tv_sec = s;
  ts.tv_nsec = ns;
  nanosleep(&ts, NULL);
}

