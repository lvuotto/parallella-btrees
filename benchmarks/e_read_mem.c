
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

  unsigned int tcore, reads, nops;
  uint32_t i;
  for (unsigned int trow = 0; trow < e_group_config.group_rows; trow++) {
    for (unsigned int tcol = 0; tcol < e_group_config.group_cols; tcol++) {
      tcore = trow*e_group_config.group_cols + tcol;
      volatile uint32_t *d32 = (uint32_t *) (BMMI_ADDRESS + 0x1000);
      volatile uint16_t *d16 = (uint16_t *) (BMMI_ADDRESS + 0x2000);
      volatile uint8_t  *d8  = (uint8_t  *) (BMMI_ADDRESS + 0x3000);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (i = 0; i < TIMES; i++)
        while (d32[i] != (0xdead0000 + i)) {}
      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t32 = (reads - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (i = 0; i < TIMES; i++)
        while (d16[i] != (0x1000 + i)) {}
      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t16 = (reads - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      for (i = 0; i < TIMES; i++)
        while (d8[i] != ((0x40 + (i & 0xff)) & 0xff)) {}
      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      for (i = 0; i < TIMES; i++)
        __asm__("nop");
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t8 = (reads - nops) / (double) (TIMES);
    }
  }
  bmmi[core].finished = E_TRUE;

  return 0;
}

