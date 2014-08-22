
#include "e-lib.h"
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "bmmi.h"


#define BMMI_ADDRESS 0x8f000000


int main () {
  e_coreid_t coreid = e_get_coreid();
  unsigned int row, col, core;
  e_coords_from_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  srand(core);

  volatile bm_msg_t *bmmi = (bm_msg_t *) BMMI_ADDRESS;
  bmmi[core].coreid = coreid;

  unsigned int tcore;
  uint32_t *d32 = (uint32_t *) 0x4000;
  uint16_t *d16 = (uint16_t *) 0x5000;
  uint8_t  *d8  = (uint8_t  *) 0x6000;
  uint32_t v32;
  uint16_t v16;
  uint8_t  v8;
  for (row = 0; row < e_group_config.group_rows; row++) {
    for (col = 0; col < e_group_config.group_cols; col++) {
      tcore = row*e_group_config.group_cols + col;
      /*if (tcore == core) continue;*/
      d32 = e_get_global_address(row, col, d32);
      d16 = e_get_global_address(row, col, d16);
      d8  = e_get_global_address(row, col, d8 );

      v32 = 0x80000000 + (rand() & 0x7fffffff);
      v16 =     0x8000 + (rand() & 0x7fff);
      v8  =       0x80 + (rand() & 0x7f);

      e_ctimer_set(E_CTIMER_0, 0xffffffff);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *d32 = v32;
      e_ctimer_stop(E_CTIMER_0);
      bmmi[core].ticks[tcore].t32 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
      
      e_ctimer_set(E_CTIMER_0, 0xffffffff);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *d16 = v16;
      e_ctimer_stop(E_CTIMER_0);
      bmmi[core].ticks[tcore].t16 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
      
      e_ctimer_set(E_CTIMER_0, 0xffffffff);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *d8 = v8;
      e_ctimer_stop(E_CTIMER_0);
      bmmi[core].ticks[tcore].t8 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
    }
  }
  bmmi[core].finished = E_TRUE;

  return 0;
}

