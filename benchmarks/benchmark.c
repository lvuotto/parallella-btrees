
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


void benchmark (e_platform_t *platform,
                e_epiphany_t *device,
                char *wrfn,
                char *rdfn);


int main () {
  e_platform_t platform;

  e_init(NULL);
  e_reset_system();
  e_get_platform_info(&platform);

  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  
  puts("Epiphany plano");
  puts("--------------");
  benchmark(&platform, &device, "e_write.srec", "e_read.srec");
  
  puts("");
  puts("Assembler inline");
  puts("----------------");
  benchmark(&platform, &device, "e_write_asm.srec", "e_read_asm.srec");
  
  e_mem_t mem;
  uint32_t a[1000];
  for (uint32_t i = 0; i < 1000; i++)
    a[i] = 0xdead0000 + i;
  e_alloc(&mem, BMMI_ADDRESS + 0x1000, sizeof(a));
  e_write(&mem, 0, 0, 0, a, sizeof(a));

  uint16_t b[2000];
  for (uint32_t i = 0; i < 2000; i++)
    b[i] = i & 1 ? 0 : 0x1000 + i;
  e_alloc(&mem, BMMI_ADDRESS + 0x2000, sizeof(b));
  e_write(&mem, 0, 0, 0, b, sizeof(b));

  uint32_t c[4000];
  for (uint32_t i = 0; i < 4000; i++)
    c[i] = i & 3 ? 0 : (0x40 + (i & 0xff)) & 0xff;
  e_alloc(&mem, BMMI_ADDRESS + 0x3000, sizeof(c));
  e_write(&mem, 0, 0, 0, c, sizeof(c));

  puts("");
  puts("Memoria directa");
  puts("---------------");
  benchmark(&platform, &device, "e_write_mem.srec", "e_read_mem.srec");
  
  e_free(&mem);
  e_close(&device);
  e_finalize();

  return 0;
}


void benchmark (e_platform_t *platform,
                e_epiphany_t *device,
                char *wrfn,
                char *rdfn)
{
  int status;
  unsigned int tcore;
  static bm_msg_t bmmi;
  e_mem_t mem;
  
  memset(&bmmi, 0, sizeof(bmmi));
  e_alloc(&mem, BMMI_ADDRESS, sizeof(bmmi));
  e_write(&mem, 0, 0, 0, &bmmi, sizeof(bmmi));
 
  e_reset_group(device);
  status = e_load(wrfn, device, 0, 0, E_TRUE);

  if (status != E_OK) {
    fprintf(stderr, "Hubo un error al cargar.\n");
    exit(1);
  }

  nano_wait(0, 10000000);

  while (E_TRUE) {
    e_read(&mem, 0, 0, 0, &bmmi, sizeof(bmmi));
    if (bmmi.finished == E_TRUE) {
      break;
    }
    nano_wait(0, 1000000);
  }

  puts("  WR");
  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      tcore = row*platform->cols + col;
      printf("    tcore#%-2u: t32=%.3f, t16=%.3f, t8=%.3f\n",
             tcore,
             bmmi.ticks[tcore].t32,
             bmmi.ticks[tcore].t16,
             bmmi.ticks[tcore].t8);
    }
  }

  e_reset_group(device);
  memset(&bmmi, 0, sizeof(bmmi));
  status = e_load(rdfn, device, 0, 0, E_TRUE);

  if (status != E_OK) {
    fprintf(stderr, "Hubo un error al cargar.\n");
    exit(1);
  }

  nano_wait(0, 10000000);

  while (E_TRUE) {
    e_read(&mem, 0, 0, 0, &bmmi, sizeof(bmmi));
    if (bmmi.finished == E_TRUE) {
      break;
    }
    nano_wait(0, 1000000);
  }
  
  puts("");
  puts("  RD");
  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      tcore = row*platform->cols + col;
      printf("    tcore#%-2u: t32=%.3f, t16=%.3f, t8=%.3f\n",
             tcore,
             bmmi.ticks[tcore].t32,
             bmmi.ticks[tcore].t16,
             bmmi.ticks[tcore].t8);
    }
  }

  e_free(&mem);
}
