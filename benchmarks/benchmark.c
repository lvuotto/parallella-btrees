
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
  memset(bmmi, 0, sizeof(bmmi));
  e_alloc(&mem, BMMI_ADDRESS, sizeof(bmmi));
  
  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_write(&mem, 0, 0, 0, bmmi, sizeof(bmmi));
  e_reset_group(&device);
  int status = e_load("e_write.srec",
                      &device,
                      0,
                      0,
                      E_TRUE);

  if (status != E_OK) {
    fprintf(stderr, "Hubo un error al cargar.\n");
    return 1;
  }

  nano_wait(0, 10000000);

  unsigned int core = 0;
  while (E_TRUE) {
    e_read(&mem,
           0,
           0,
           (off_t) ((char *) &bmmi[core] - (char *) bmmi),
           &bmmi[core],
           sizeof(bm_msg_t));
    if (bmmi[core].finished == E_TRUE) {
      break;
    }
    nano_wait(0, 1000000);
  }

  unsigned int tcore;
  puts("Write:");
  printf("core#%u:\n", core);
  for (unsigned int row = 0; row < platform.rows; row++) {
    for (unsigned int col = 0; col < platform.cols; col++) {
      tcore = row*platform.cols + col;
      printf("  tcore#%-2u: t32=%.3f, t16=%.3f, t8=%.3f\n",
             tcore,
             bmmi[core].ticks[tcore].t32,
             bmmi[core].ticks[tcore].t16,
             bmmi[core].ticks[tcore].t8);
    }
  }

  e_reset_group(&device);
  status = e_load("e_read.srec",
                  &device,
                  0,
                  0,
                  E_TRUE);

  if (status != E_OK) {
    fprintf(stderr, "Hubo un error al cargar.\n");
    return 1;
  }

  nano_wait(0, 10000000);

  while (E_TRUE) {
    e_read(&mem,
           0,
           0,
           (off_t) ((char *) &bmmi[core] - (char *) bmmi),
           &bmmi[core],
           sizeof(bm_msg_t));
    if (bmmi[core].finished == E_TRUE) {
      break;
    }
    nano_wait(0, 1000000);
  }
  
  puts("");
  puts("Read:");
  printf("core#%u:\n", core);
  for (unsigned int row = 0; row < platform.rows; row++) {
    for (unsigned int col = 0; col < platform.cols; col++) {
      tcore = row*platform.cols + col;
      printf("  tcore#%-2u: t32=%.3f, t16=%.3f, t8=%.3f\n",
             tcore,
             bmmi[core].ticks[tcore].t32,
             bmmi[core].ticks[tcore].t16,
             bmmi[core].ticks[tcore].t8);
    }
  }

  e_close(&device);
  e_free(&mem);
  e_finalize();

  return 0;
}

