
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
  uint32_t *b32 = (uint32_t *) 0x4000;
  uint16_t *b16 = (uint16_t *) 0x4000;
  uint8_t  *b8  = (uint8_t  *) 0x4000;
  for (unsigned int trow = 0; trow < e_group_config.group_rows; trow++) {
    for (unsigned int tcol = 0; tcol < e_group_config.group_cols; tcol++) {
      tcore = trow*e_group_config.group_cols + tcol;
      /*if (tcore == core) continue;*/
      volatile uint32_t *d32 = e_get_global_address(trow, tcol, b32);
      volatile uint16_t *d16 = e_get_global_address(trow, tcol, b16);
      volatile uint8_t  *d8  = e_get_global_address(trow, tcol, b8 );

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

      /* READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._read_loop_32: "
                           "ldr r34, [r32], #4\n"
                           "sub r33, r33, #1\n"
                           "bne ._read_loop_32\n"
                           : /* no output */
                           : "r"(d32), "i"(TIMES)
                           : "r32", "r33");

      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);

      /* DUMMY READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._dummy_read_loop_32: "
                           "nop\n"
                           "sub r33, r33, #1\n"
                           "bne ._dummy_read_loop_32\n"
                           : /* no output */
                           : "r"(d32), "i"(TIMES)
                           : "r32", "r33");

      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t32 = (reads - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

      /* READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._read_loop_16: "
                           "ldrh r34, [r32], #4\n"
                           "sub r33, r33, #1\n"
                           "bne ._read_loop_16\n"
                           : /* no output */
                           : "r"(d16), "i"(TIMES)
                           : "r32", "r33");

      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);

      /* DUMMY READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._dummy_read_loop_16: "
                           "nop\n"
                           "sub r33, r33, #1\n"
                           "bne ._dummy_read_loop_16\n"
                           : /* no output */
                           : "r"(d16), "i"(TIMES)
                           : "r32", "r33");

      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t16 = (reads - nops) / (double) (TIMES);

      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
      e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

      /* READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._read_loop_8: "
                           "ldrb r34, [r32], #4\n"
                           "sub r33, r33, #1\n"
                           "bne ._read_loop_8\n"
                           : /* no output */
                           : "r"(d8), "i"(TIMES)
                           : "r32", "r33");
      
      e_ctimer_stop(E_CTIMER_0);
      reads = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);

      /* DUMMY READ LOOP */
      __asm__ __volatile__("mov r33, %1\n"
                           "mov r32, %0\n"
                           "._dummy_read_loop_8: "
                           "nop\n"
                           "sub r33, r33, #1\n"
                           "bne ._dummy_read_loop_8\n"
                           : /* no output */
                           : "r"(d8), "i"(TIMES)
                           : "r32", "r33");
       
      e_ctimer_stop(E_CTIMER_0);
      nops = E_CTIMER_MAX - e_ctimer_get(E_CTIMER_0);
      bmmi[core].ticks[tcore].t8 = (reads - nops) / (double) (TIMES);
    }
  }
  bmmi[core].finished = E_TRUE;

  return 0;
}
