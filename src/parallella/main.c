#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e-hal.h>
#include "btmi.h"


#define BTMI_ADDRESS 0x1000000
#define E_CORES 16


int main()
{
  e_platform_t platform;
  e_mem_t mem;

  e_init("b-tree.hdf");
  e_reset_system();
  e_get_platform_info(&platform);
  e_alloc(&mem, BTMI_ADDRESS, sizeof(btmi));
  
  static b_tree_msg_t btmi[16];
  memset(mem, 0, sizeof(btmi));

  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_write(&mem, 0, 0, 0, btmi, sizeof(btmi));
  e_reset_group(&device);
  int status = e_load_group("e_b-tree.srec",
                            &device,
                            0, 0,
                            platform.rows, platform.cols,
                            E_TRUE);

  if (status != E_OK) {
    fputs(stderr, "Hubo problemas cargando el ejecutable.");
    exit(1);
  }

  nano_wait(0, 10000000);
  
  int core;
  for (int row = 0; row < platform.rows; rows++) {
    for (int col = 0; col < platform.cols; col++) {
      core = row*platform.cols + col;
      do {
        e_read(&mem,
               0,
               0,
               (off_t) ((char *) &btmi[core] - (char *) btmi),
               &btmi[core],
               sizeof(btmi[core]);
        nano_wait(0, 1000000);
      } while (btmi[core].status == B_JOB_TO_DO);
    }
  }
  
  e_close(&device);
  e_free(&mem);
  e_finalize();

  return 0;
}
