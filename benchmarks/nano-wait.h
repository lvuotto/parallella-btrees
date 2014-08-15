
#include <unistd.h>
#include <time.h>
#include <stdint.h>


static inline void nano_wait (uint32_t s, uint32_t ns) {
  struct timespec ts;
  ts.tv_sec = s;
  ts.tv_nsec = ns;
  nanosleep(&ts, NULL);
}

