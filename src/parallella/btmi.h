#ifndef __BTMI_H__

#define __BTMI_H__


#include <stdint.h>


typedef enum   b_status_e   b_status_t;
typedef enum   b_job_e      b b_job_t;
typedef struct b_response_s b_response_t;
typedef struct b_msg_s      b_msg_t;


enum b_status_e {
  B_OK,
  B_STAND_BY,
  B_JOB_TO_DO,
  B_NOT_FOUND,
  B_UNRECOGNIZED
};

enum b_job_e {
  B_NO_JOB,
  B_INSERT,
  B_FIND
};

struct b_response_s {
  b_status_t   s;
  uint32_t     v;
} __attribute__((packed, aligned(4)));

struct b_msg_s {
  b_status_t   status;
  b_job_t      job;
  b_response_t response;
} __attribute__((packed, aligned(4)));


#endif  /* __BTMI_H__ */
