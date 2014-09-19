#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e-hal.h>

#include "btmi.h"
#include "nano-wait.h"
#include "b-tree.h"
#include "queue.h"

#define BTMI_ADDRESS 0x01000000
#define B_TREE       0x01001000
#define E_CORES              16

#define log(s) fputs(s "\n", stderr)


void share(const b_tree_t *tree, e_mem_t *mem);
b_status_t * b_find_parallel(e_platform_t *platform,
                             e_mem_t *mem,
                             b_msg_t *msg,
                             /*b_tree_t *tree,*/
                             const b_key_t *keys);


int main()
{
  e_platform_t platform;

  if (e_init(NULL) != E_OK) {
    log("Error al inicializar.");
    exit(1);
  }
  e_reset_system();
  e_get_platform_info(&platform);
  
  e_mem_t mem, tree_mem;
  static b_msg_t btmi[16];
  memset(btmi, 0, sizeof(btmi));
  e_alloc(&mem, BTMI_ADDRESS, sizeof(btmi));
  /*e_alloc(&tree_mem, B_TREE, 0x1000000);*/

  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_write(&mem, 0, 0, 0, btmi, sizeof(btmi));
  e_reset_group(&device);
  /*int status = e_load_group("e_b-tree.srec",
                            &device,
                            0, 0,
                            platform.rows, platform.cols,
                            E_TRUE);*/
  int status = e_load("e_b-tree.srec", &device, 0, 0, E_TRUE);

  if (status != E_OK) {
    log("Hubo problemas cargando el ejecutable.");
    exit(1);
  }

  nano_wait(0, 10000000);
 
  /* COMIENZO busqueda en paralelo */
  b_tree_t *tree;
  tree = b_new();
  int a[16];
  for (int i = 0; i < 16; i++) {
    a[i] = i + 1;
    b_add(tree, a[i]);
  }
  /*share(tree, &tree_mem);*/
  /*b_status_t *response = b_find_parallel(&platform, &mem, btmi, tree, a);*/
  
  log("Realizando la busqueda...");
  b_status_t *response = b_find_parallel(&platform, &mem, btmi, a);
  log("Busqueda completada.");
  for (int i = 0; i < 16; i++) {
    printf("%d: %d\n", a[i], response[i]);
  }
  free(response);
  b_delete(tree);
  /* FIN busqueda en paralelo */
  

  /*unsigned int core;
  for (unsigned int row = 0; row < platform.rows; row++) {
    for (unsigned int col = 0; col < platform.cols; col++) {
      core = row*platform.cols + col;
      do {
        e_read(&mem,
               0,
               0,
               (off_t) ((char *) &btmi[core] - (char *) btmi),
               &btmi[core],
               sizeof(btmi[core]));
        nano_wait(0, 1000000);
      } while (btmi[core].status == B_JOB_TO_DO);
    }
  }*/
  
  e_close(&device);
  e_free(&tree_mem);
  e_free(&mem);
  e_finalize();

  return 0;
}


void share(const b_tree_t *tree, e_mem_t *mem)
{
  queue *q = queue_new();
  enqueue(q, tree->root);
  
  off_t pos = B_TREE;
  while (!empty(q)) {
    b_node_t *n = dequeue(q);
    for (int i = 0; i <= n->used_keys; i++) {
      if (n->children[i] != NULL) {
        enqueue(q, n->children[i]);
      }
    }
    e_write(mem, 0, 0, pos, n, sizeof(b_node_t));
    pos += sizeof(b_node_t);
  }

  queue_delete(q);
}


/**
 * Proxy para cargar los datos y realizar las busquedas.
 *
 * TODO:
 *  - Ver algo sobre si el árbol está o no en memoria.
 **/
b_status_t * b_find_parallel(e_platform_t *platform,
                             e_mem_t *mem,
                             b_msg_t *msg,
                             /*b_tree_t *tree,*/
                             const b_key_t *keys)
{
  b_status_t *response = (b_status_t *) malloc(16 * sizeof(b_status_t));
  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      /*msg[core].job = B_FIND;*/
      msg[core].status = B_JOB_TO_DO;
      /*msg[core].param = keys[core];*/
    }
  }
  e_write(mem, 0, 0, 0, msg, 16 * sizeof(b_msg_t));

  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      logf("core#%u:", core);
      off_t offset = (off_t) ((char *) &msg[core] - (char *) msg);
      do {
        int s = e_read(mem, 0, 0, offset, &msg[core], sizeof(msg[core]));
        if (s == E_ERR) {
          log("error en `e_read`.");
          exit(1);
        }
        nano_wait(0, 1000000);
        logf("%u %u %u %u %u\n",
             msg[core].status,
             msg[core].job,
             msg[core].param,
             msg[core].response.s,
             msg[core].response.v);
      } while (msg[core].status == B_JOB_TO_DO);
      log("");
      response[core] = msg[core].response.s;
    }
  }

  return response;
}
