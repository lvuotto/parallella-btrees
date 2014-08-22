
#ifndef __COMPARTIDO_H__

#define __COMPARTIDO_H__


#include <stdint.h>


typedef struct msg_s msg_t;


struct msg_s {
  uint32_t dormir_0;
  uint32_t dormir_1000;
  uint32_t dormir_5000;
  uint32_t dormir_10000;
  uint32_t finalizo;
} __attribute__((packed));


#endif  /* __COMPARTIDO__ */

