
#include "e-lib.h"
#include <stdlib.h>
#include <stdint.h>

#include "compartido.h"


#define MSG_ADDRESS 0x8f000000


int main () {
  e_coreid_t coreid = e_get_coreid();
  unsigned int row, col, core;
  e_coords_from_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  volatile msg_t *msg = (msg_t *) MSG_ADDRESS;

  e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
  e_wait(E_CTIMER_1, 0);
  e_ctimer_stop(E_CTIMER_0);
  msg[core].dormir_0 = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
  
  e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
  e_wait(E_CTIMER_1, 1000);
  e_ctimer_stop(E_CTIMER_0);
  msg[core].dormir_1000 = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);

  e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
  e_wait(E_CTIMER_1, 5000);
  e_ctimer_stop(E_CTIMER_0);
  msg[core].dormir_5000 = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);

  e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
  e_wait(E_CTIMER_1, 10000);
  e_ctimer_stop(E_CTIMER_0);
  msg[core].dormir_10000 = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);

  msg[core].finalizo = E_TRUE;

  return 0;
}

