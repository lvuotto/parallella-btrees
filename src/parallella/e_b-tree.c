#include "e-lib.h"
#include "btmi.h"


#define BTMI_ADDRESS 0x8f000000
#define B_TREE       0x8f001000


int main()
{
  e_coreid_t coreid = e_get_coreid();
  unsigned int row, col, core;

  e_coords_from_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  volatile b_msg_t *msg = (b_msg_t *) BTMI_ADDRESS;
  
  msg[core].status     = 0xdead;
  msg[core].job        = 0xdead;
  msg[core].param      = 0xdead;
  msg[core].response.s = 0xdead;
  msg[core].response.v = 0xdead;

  return 0;
}
