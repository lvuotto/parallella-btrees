
#include "e-lib.h"
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "bmmi.h"


#define BMMI_ADDRESS 0x8f000000
#define TIMES        1000


int main () {
  e_coreid_t coreid = e_get_coreid();
  unsigned int row, col, core;
  e_coords_from_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  srand(core);

  volatile bm_msg_t *bmmi = (bm_msg_t *) BMMI_ADDRESS;
  bmmi[core].coreid = coreid;

  unsigned int tcore, writes, nops;
  uint32_t *b32 = (uint32_t *) 0x4000;
  uint16_t *b16 = (uint16_t *) 0x5000;
  uint8_t  *b8  = (uint8_t  *) 0x6000;
  uint32_t v32;
  uint16_t v16;
  uint8_t  v8;
  for (unsigned int trow = 0; trow < e_group_config.group_rows; trow++) {
    for (unsigned int tcol = 0; tcol < e_group_config.group_cols; tcol++) {
      tcore = trow*e_group_config.group_cols + tcol;
      /*if (tcore == core) continue;*/
      uint32_t *d32 = e_get_global_address(trow, tcol, b32);
      uint16_t *d16 = e_get_global_address(trow, tcol, b16);
      uint8_t  *d8  = e_get_global_address(trow, tcol, b8 );

      v32 = 0x80000000 + (rand() & 0x7fffffff);
      v16 =     0x8000 + (rand() & 0x7fff);
      v8  =       0x80 + (rand() & 0x7f);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (int i = 0; i < TIMES; i++)
        e_write(&e_group_config, &v32, trow, tcol, d32 + i, sizeof(v32));
      e_ctimer_stop(E_CTIMER_0);
      writes = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (int i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t32 = (writes - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (int i = 0; i < TIMES; i++)
        e_write(&e_group_config, &v16, trow, tcol, d16 + i, sizeof(v16));
      e_ctimer_stop(E_CTIMER_0);
      writes = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (int i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t16 = (writes - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (int i = 0; i < TIMES; i++)
        e_write(&e_group_config, &v8, trow, tcol, d8 + 4*i, sizeof(v8));
      e_ctimer_stop(E_CTIMER_0);
      writes = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (int i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t8 = (writes - nops) / (double) (TIMES);
    }
  }
  bmmi[core].finished = E_TRUE;

  return 0;
}

