
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <e-hal.h>
#include "nano-wait.h"

#include "bmmi.h"


#define BMMI_ADDRESS 0x1000000
#define E_CORES 16


/*

static inline uint32_t rdtsc32 (void) {
#if defined(__GNUC__) && defined(__ARM_ARCH_7A__)
  uint32_t r = 0;
  asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(r) );
  return r;
#else
# error Unsupported architecture/compiler!
#endif
}

*/


int main () {
  e_platform_t platform;
  e_mem_t mem;

  e_init(NULL);
  e_reset_system();
  e_get_platform_info(&platform);

  static bm_msg_t bmmi[E_CORES];
  memset(&mem, 0, sizeof(bmmi));
  e_alloc(&mem, BMMI_ADDRESS, sizeof(bmmi));
  
  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_write(&mem, 0, 0, 0, bmmi, sizeof(bmmi));
  e_reset_group(&device);
  e_load_group("e_benchmarks.srec", &device, 0, 0, platform.rows, platform.cols, E_TRUE);

  nano_wait(0, 10000000);

  unsigned int core;
  for (unsigned int row = 0; row < platform.rows; row++) {
    for (unsigned int col = 0; col < platform.cols; col++) {
      core = row*platform.cols + col;
      while (E_TRUE) {
        e_read(&mem,
               0,
               0,
               (off_t) ((char *) &bmmi[core] - (char *) bmmi),
               &bmmi[core],
               sizeof(bmmi[core]));
        if (bmmi[core].status != E_FALSE)
          break;
        nano_wait(0, 1000000);
      }
    }
  }
  
  unsigned int tcore;
  for (core = 0; core < platform.rows*platform.cols; core++) {
    printf("core#%u:\n", core);
    for (unsigned int row = 0; row < platform.rows; row++) {
      for (unsigned int col = 0; col < platform.cols; col++) {
        tcore = row*platform.cols + col;
        printf("  tcore#%u: t32=%u, t16=%u, t8=%u\n",
               tcore,
               bmmi[core].ticks[tcore].t32,
               bmmi[core].ticks[tcore].t16,
               bmmi[core].ticks[tcore].t8);
      }
    }
  }
  
  e_close(&device);
  e_free(&mem);
  e_finalize();

  return 0;
}

