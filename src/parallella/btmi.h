
#ifndef __BTMI_H__

#define __BTMI_H__


#include <stdint.h>


typedef enum   b_tree_status_e b_tree_status_t;
typedef enum   b_tree_job_e    b_tree_job_t;
typedef struct b_tree_msg_s    b_tree_msg_t;


enum b_tree_status_e {
  B_STAND_BY,
  B_JOB_TO_DO,
  B_WORKING,
  B_MASTER_JOB
};

enum b_tree_job_e {
  B_NO_JOB,
  B_INSERT,
  B_FIND
};

struct b_tree_msg_s {
  b_tree_status_t  status:2;
  b_tree_job_t     job:6;
  uint32_t         response;
} __attribute__((packed, aligned(4)));


#endif  /* __BTMI_H__ */

