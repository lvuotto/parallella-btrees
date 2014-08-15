
#ifndef __BMI_H__

#define __BMI_H__


#include <stdint.h>


typedef struct bm_ticks_s bm_ticks_t;
typedef struct bm_msg_s   bm_msg_t;


struct bm_ticks_s {
  uint32_t t32;
  uint32_t t16;
  uint32_t t8;
};

struct bm_msg_s {
  uint32_t    coreids[16];
  bm_ticks_t  ticks[16];
  uint32_t    status[16];
};


#endif  /* __BMI_H__ */

