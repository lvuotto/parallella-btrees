
#include "e-lib.h"
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "bmmi.h"


#define BMMI_ADDRESS 0x8f000000


int main () {
  e_coreid_t coreid = e_get_coreid();
  uint32_t row, col, core;
  e_coords_form_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  srand(time(NULL));
  
  uint32_t *tca32, m32, start, end;
  uint8_t *tca8, m8;
  uint16_t *tca16, m16;
  volatile bm_msg_t *msg = (bm_msg_t *) BMMI_ADDRESS;
  tca32 = (uint32_t *) 0x4000;
  tca16 = (uint16_t *) 0x4100;
  tca8  = (uint8_t  *) 0x4200;
  tca32 = 0;
  tca16 = 0;
  tca8  = 0;
  for (col = 0; col < e_group_config.group_cols; col++) {
    for (row = 0; row < e_group_config.group_rows; row++) {
      tca32 = e_get_global_address(row, col, tca32);
      tca16 = e_get_global_address(row, col, tca16);
      tca8  = e_get_global_address(row, col, tca8 );

      m32 = 1 + rand() % 0xffffffff;
      m16 = 1 + rand() % 0xffff;
      m8  = 1 + rand() % 0xff;

      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *tca32 = m32;
      e_ctimer_stop(E_CTIMER_0);
      msg->ticks[core].t32 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
      
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *tca16 = m16;
      e_ctimer_stop(E_CTIMER_0);
      msg->ticks[core].t16 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
      
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
      *tca8 = m8;
      e_ctimer_stop(E_CTIMER_0);
      msg->ticks[core].t8 = 0xffffffff - e_ctimer_get(E_CTIMER_0);
    }
  }

  msg->status[core] = E_TRUE;

  return 0;
}

