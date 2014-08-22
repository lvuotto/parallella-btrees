
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <e-hal.h>

#include "nano-wait.h"
#include "compartido.h"


#define MSG_ADDRESS 0x1000000
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

  static msg_t msg[E_CORES];
  memset(msg, 0, sizeof(msg));
  e_alloc(&mem, MSG_ADDRESS, sizeof(msg));
  
  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_write(&mem, 0, 0, 0, msg, sizeof(msg));
  e_reset_group(&device);
  int status = e_load_group("e_tiempos.srec",
                            &device,
                            0,
                            0,
                            platform.rows,
                            platform.cols,
                            E_TRUE);

  if (status != E_OK) {
    fprintf(stderr, "Hubo un error al cargar.\n");
    return 1;
  }

  nano_wait(0, 10000000);

  unsigned int core;
  for (unsigned int row = 0; row < platform.rows; row++) {
    for (unsigned int col = 0; col < platform.cols; col++) {
      core = row*platform.cols + col;
      while (E_TRUE) {
        e_read(&mem,
               0,
               0,
               (off_t) ((char *) &msg[core] - (char *) msg),
               &msg[core],
               sizeof(msg[core]));
        if (msg[core].finalizo == E_TRUE) {
          break;
        }
        nano_wait(0, 1000000);
      }
    }
  }

  for (core = 0; core < platform.rows*platform.cols; core++) {
    printf("core#%u:\n"
           "  dormir_0: %u\n"
           "  dormir_1000: %u\n"
           "  dormir_5000: %u\n"
           "  dormir_10000: %u\n",
           core,
           msg[core].dormir_0,
           msg[core].dormir_1000,
           msg[core].dormir_5000,
           msg[core].dormir_10000);

  }

  e_close(&device);
  e_free(&mem);
  e_finalize();

  return 0;
}

