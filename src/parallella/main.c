#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e-hal.h>
#include <time.h>

#include "btmi.h"
#include "nano-wait.h"
#include "b-tree.h"

#define DRAM_SIZE   0x02000000 /* 32MB */
#define B_TREE      0x00000000
#define MSG_ADDRESS 0x00004000 /* 16KB, memoria local. */
#define E_CORES             16

#define W_1ms          1000000
#define W_10ms        10000000
#define W_100ms      100000000

#define TEST_SIZE    (1 << 16) 


off_t share(const b_tree_t *tree, e_mem_t *mem);
b_node_t * e_share(e_mem_t *mem, off_t *pos, b_node_t *n, b_node_t *parent);
b_status_t * b_find_parallel(e_platform_t *platform,
                             e_epiphany_t *device,
                             e_mem_t *mem,
                             b_msg_t *msg,
                             const b_key_t *keys);


int main()
{
#ifdef E_DBG_ON
  e_set_host_verbosity(H_D1);
  e_set_loader_verbosity(L_D1);
#endif

  e_platform_t platform;

  if (e_init(NULL) != E_OK) {
    printf("Error al inicializar.");
    exit(1);
  }
  e_reset_system();
  e_get_platform_info(&platform);

  e_mem_t mem, tree_mem;
  static b_msg_t btmi[16];
  memset(btmi, 0, sizeof(btmi));
  if (e_alloc(&mem, B_TREE, DRAM_SIZE - B_TREE) != E_OK) {
    printf("error al alocar (msjs)");
    exit(1);
  }
  
  e_epiphany_t device;
  e_open(&device, 0, 0, platform.rows, platform.cols);
  e_reset_group(&device);
  int status = e_load_group("e_b-tree.srec",
                            &device,
                            0, 0,
                            platform.rows, platform.cols,
                            E_FALSE);

  if (status != E_OK) {
    printf("Hubo problemas cargando el ejecutable.");
    exit(1);
  }

  /* COMIENZO busqueda en paralelo */
  b_tree_t *tree;
  tree = b_new();
  int b[E_CORES];
  for (int i = 0; i < TEST_SIZE; i++) {
    b_add(tree, i+1);
  }
  share(tree, &mem);

  clock_t dt;
  double tiempo;
  printf("Realizando la busqueda...");
  dt = clock();
  for (int total = 0; total < TEST_SIZE; total += E_CORES) {
    for (int j = 0; j < E_CORES; j++)
      b[j] = 1+j + total;
    b_status_t *response = b_find_parallel(&platform, &device, &mem, btmi, b);
    free(response);
  }
  dt = clock() - dt;
  printf("Busqueda completada.");
  tiempo = ((double) dt) / CLOCKS_PER_SEC;
  printf("Tiempo total: %.5fs\n", tiempo);
  b_delete(tree);
  /* FIN busqueda en paralelo */

  e_close(&device);
  e_free(&tree_mem);
  e_free(&mem);
  e_finalize();

  return 0;
}


off_t share(const b_tree_t *tree, e_mem_t *mem)
{
#ifdef E_DBG_ON
  e_set_host_verbosity(H_D4);
#endif
  
  /**
   * TODO:
   *  - Ver de reprogramar el árbol de modo de que no haya que compartirlo
   *    al epiphany antes de usarlo, sino que se comparta cada vez que se
   *    hace un cambio.
   **/
  /**
   * Se debe compactificar el árbol en el área de memoria compartida.
   * `cn` es el nodo a escribir con los punteros arreglados para mantener
   * coherencia.
   **/
  
  off_t pos = B_TREE + sizeof(*tree);
  b_tree_t c = *tree;
  c.root = e_share(mem, &pos, tree->root, NULL);
  e_write(mem, 0, 0, B_TREE, &c, sizeof(c));

#ifdef E_DBG_ON
  e_set_host_verbosity(H_D1);
#endif
  
  return pos;
}


b_node_t * e_share(e_mem_t *mem, off_t *pos, b_node_t *n, b_node_t *parent)
{
  b_node_t clone = *n;
  clone.parent = parent;
  off_t old_pos = *pos;
  b_node_t *actual = (b_node_t *) (mem->ephy_base + old_pos);
  *pos += sizeof(clone);
  for (int i = 0; i <= n->used_keys && n->children[i] != NULL; i++) {
    clone.children[i] = e_share(mem, pos, n->children[i], actual);
  }
  e_write(mem, 0, 0, old_pos, &clone, sizeof(clone));

  return actual;
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
                             const b_key_t *keys)
{
  b_status_t *response = (b_status_t *) malloc(16 * sizeof(b_status_t));
  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      msg[core].job = B_FIND;
      msg[core].status = B_JOB_TO_DO;
      msg[core].param = keys[core];
      /*msg[core].response.s = 0;
      msg[core].response.v = 0;*/
      e_write(device,
              row, col,
              MSG_ADDRESS,
              &msg[core], sizeof(msg[core]));
    }
  }
  e_write(mem, 0, 0, 0, msg, 16 * sizeof(b_msg_t));
  e_start_group(device);

  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      off_t offset = (off_t) ((char *) &msg[core] - (char *) msg);
      do {
        int s = e_read(mem, 0, 0, offset, &msg[core], sizeof(msg[core]));
        if (s == E_ERR) {
          printf("error en `e_read`.");
          exit(1);
        }
      } while (msg[core].status == B_JOB_TO_DO);
      response[core] = msg[core].response.s;
    }
  }

  return response;
}
