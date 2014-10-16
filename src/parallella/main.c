#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e-hal.h>

#include "btmi.h"
#include "nano-wait.h"
#include "b-tree.h"
#include "queue.h"


#define DRAM_SIZE    0x02000000 /* 32MB */
#define BTMI_ADDRESS 0x01000000
#define B_TREE       0x01001000
#define E_CORES              16

#define W_1ms           1000000
#define W_10ms         10000000
#define W_100ms       100000000

#define TEST_SIZE            16

#define log(s)         fputs(s "\n", stderr)
#define logf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)


#ifndef E_DBG_ON
# define E_DBG_ON 1
#endif


void share(const b_tree_t *tree, e_mem_t *mem);
b_status_t * b_find_parallel(e_platform_t *platform,
                             e_epiphany_t *device,
                             e_mem_t *mem,
                             b_msg_t *msg,
                             /*b_tree_t *tree,*/
                             const b_key_t *keys);


int main()
{
#ifdef E_DBG_ON
  e_set_host_verbosity(H_D1);
  e_set_loader_verbosity(L_D1);
#endif

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
  if (e_alloc(&mem, BTMI_ADDRESS, DRAM_SIZE - BTMI_ADDRESS) != E_OK) {
    log("error al alocar (msjs)");
    exit(1);
  }
  /*if (e_alloc(&tree_mem, B_TREE, DRAM_SIZE - B_TREE) != E_OK) {
    log("error al alocar (arbol)");
    exit(1);
  }*/
  
  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_reset_group(&device);
  int status = e_load_group("e_b-tree.srec",
                            &device,
                            0, 0,
                            platform.rows, platform.cols,
                            E_FALSE);

  if (status != E_OK) {
    log("Hubo problemas cargando el ejecutable.");
    exit(1);
  }

  /* COMIENZO busqueda en paralelo */
  b_tree_t *tree;
  tree = b_new();
  int a[TEST_SIZE], b[TEST_SIZE];
  for (int i = 0; i < TEST_SIZE; i++) {
    a[i] = i + 1;
    b[i] = i + 1 + TEST_SIZE*(i==0);
    b_add(tree, a[i]);
  }
  share(tree, &mem);

  log("Realizando la busqueda...");
  b_status_t *response = b_find_parallel(&platform, &device, &mem, btmi, b);
  log("Busqueda completada.");
  for (int i = 0; i < TEST_SIZE; i++) {
    printf("%d: %p\n", b[i], (void *) response[i]);
  }
  free(response);
  b_delete(tree);
  /* FIN busqueda en paralelo */

  e_close(&device);
  e_free(&tree_mem);
  e_free(&mem);
  e_finalize();

  return 0;
}


void share(const b_tree_t *tree, e_mem_t *mem)
{
#ifdef E_DBG_ON
  e_set_host_verbosity(H_D4);
#endif
  
  off_t pos = 0x1000;
  b_tree_t ct = *tree;
  ct.root = (b_node_t *) (mem->ephy_base + pos + sizeof(ct.root));
  e_write(mem, 0, 0, pos, &ct, sizeof(ct));
  pos += sizeof(ct);

  if (tree->root == NULL)
    return;

  /**
   * Se debe compactificar el árbol en el área de memoria compartida.
   * `cn` es el nodo a escribir con los punteros arreglados para mantener
   * coherencia.
   **/
  queue *q = queue_new();
  enqueue(q, tree->root);
  while (!queue_empty(q)) {
    b_node_t *n = dequeue(q);
    b_node_t cn = *n;
    for (int i = 0; i <= n->used_keys; i++) {
      if (n->children[i] != NULL) {
        enqueue(q, n->children[i]);
        cn.children[i] = (b_node_t *) (mem->ephy_base +
                                       pos +
                                       i * sizeof(b_node_t));
      }
    }
    e_write(mem, 0, 0, pos, &cn, sizeof(cn));
    pos += sizeof(cn);
  }

  queue_delete(q);

#ifdef E_DBG_ON
  e_set_host_verbosity(H_D1);
#endif
}


/**
 * Proxy para cargar los datos y realizar las busquedas.
 *
 * TODO:
 *  - Ver algo sobre si el árbol está o no en memoria.
 **/
b_status_t * b_find_parallel(e_platform_t *platform,
                             e_epiphany_t *device,
                             e_mem_t *mem,
                             b_msg_t *msg,
                             /*b_tree_t *tree,*/
                             const b_key_t *keys)
{
  b_status_t *response = (b_status_t *) malloc(16 * sizeof(b_status_t));
  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      msg[core].job = B_FIND;
      msg[core].status = B_JOB_TO_DO;
      msg[core].param = keys[core];
    }
  }
  e_write(mem, 0, 0, 0, msg, 16 * sizeof(b_msg_t));
  nano_wait(0, W_10ms);
  e_start_group(device);

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
        nano_wait(0, W_1ms);
        logf("%u %u %u %p %u\n",
             msg[core].status,
             msg[core].job,
             msg[core].param,
             (void *) msg[core].response.s,
             msg[core].response.v);
      } while (msg[core].status == B_JOB_TO_DO);
      response[core] = msg[core].response.s;
    }
  }

  return response;
}
