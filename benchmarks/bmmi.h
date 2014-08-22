
#ifndef __BMMI_H__

#define __BMMI_H__


#include <stdint.h>


typedef struct bm_ticks_s bm_ticks_t;
typedef struct bm_msg_s   bm_msg_t;


struct bm_ticks_s {
  uint32_t t32;
  uint32_t t16;
  uint32_t t8;
} __attribute__((packed));

struct bm_msg_s {
  uint32_t    coreid;
  bm_ticks_t  ticks[16];
  uint32_t    finished;
} __attribute__((packed));


#endif  /* __BMMI_H__ */

