#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e-hal.h>

#include "btmi.h"
#include "nano-wait.h"
#include "b-tree.h"

#define DRAM_SIZE    0x02000000 /* 32MB */
#define BTMI_ADDRESS 0x00000000
#define B_TREE       0x00001000
#define E_CORES              16

#define W_1ms           1000000
#define W_10ms         10000000
#define W_100ms       100000000

const int B_MAGIC0 = 0x7a89;

#define TEST_SIZE    0x00010000

#define log(s)         fputs(s "\n", stderr)
#define logf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)


/*#ifndef E_DBG_ON
# define E_DBG_ON 1
#endif*/


size_t mem_usage(const b_tree_t *t);
size_t quantity(const b_node_t *n);
off_t share(const b_tree_t *tree, e_mem_t *mem);
b_node_t * e_share(e_mem_t *mem, off_t *pos, b_node_t *n, b_node_t *parent);
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
  int b[E_CORES];
  for (int i = 0; i < TEST_SIZE; i++) {
    b_add(tree, i+1);
  }
  size_t w = mem_usage(tree);
  int magic = B_MAGIC0;
  e_write(&mem,
          0, 0,
          B_TREE + w - sizeof(b_node_t) + B_CHILDREN_OFFSET,
          &magic, sizeof(magic));
  size_t sarasa = share(tree, &mem);
  do {
    e_read(&mem,
           0, 0,
           B_TREE + w - sizeof(b_node_t) + B_CHILDREN_OFFSET,
           &magic, sizeof(magic));
    nano_wait(0, W_10ms);
    printf("%d == %d ? %d\n", magic, B_MAGIC0, magic == B_MAGIC0);
  } while (magic == B_MAGIC0);

  log("Realizando la busqueda...");
  for (int total = 0; total < TEST_SIZE; total += E_CORES) {
    for (int j = 0; j < E_CORES; j++)
      b[j] = 1+j + total;
    b_status_t *response = b_find_parallel(&platform, &device, &mem, btmi, b);
    /*for (int j = 0; j < E_CORES; j++)
      printf("%d: %p\n", b[j], (void *) response[j]);*/
    free(response);
  }
  log("Busqueda completada.");
  b_delete(tree);
  /* FIN busqueda en paralelo */

  e_close(&device);
  e_free(&tree_mem);
  e_free(&mem);
  e_finalize();

  return 0;
}


size_t mem_usage(const b_tree_t *t)
{
  return sizeof(t) + sizeof(b_node_t) * quantity(t->root);
}


size_t quantity(const b_node_t *n)
{
  if (n == NULL)
    return 0;

  size_t r = 1;
  for (int i = 0; i <= n->used_keys; i++) {
    r += quantity(n->children[i]);
  }
  return r;
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
  e_write(mem, 0, 0, pos, &B_MAGIC0, sizeof(int));

#ifdef E_DBG_ON
  e_set_host_verbosity(H_D1);
#endif
  
  return pos;
}


b_node_t * e_share(e_mem_t *mem, off_t *pos, b_node_t *n, b_node_t *parent)
{
  b_node_t clone = *n;
  clone.parent = parent;
  /*e_write(mem, 0, 0, *pos, &clone, sizeof(clone));*/
  off_t old_pos = *pos;
  b_node_t *actual = (b_node_t *) (mem->ephy_base + old_pos);
  *pos += sizeof(clone);
  for (int i = 0; i <= n->used_keys && n->children[i] != NULL; i++) {
    clone.children[i] = e_share(mem, pos, n->children[i], actual);
    /*size_t c = (size_t) e_share(mem, pos, n->children[i], actual);
    printf("pos = %d, epos = %d, c = %p\n",
           old_pos,
           old_pos + B_CHILDREN_OFFSET + i * sizeof(b_node_t *),
           (void *) c);
    e_write(mem,
            0, 0,
            old_pos + B_CHILDREN_OFFSET + i * sizeof(b_node_t *),
            &c, sizeof(c));*/
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
  nano_wait(0, W_1ms);
  e_start_group(device);

  for (unsigned int row = 0; row < platform->rows; row++) {
    for (unsigned int col = 0; col < platform->cols; col++) {
      unsigned int core = row * platform->cols + col;
      /*logf("core#%u: ", core);*/
      /*off_t offset = (off_t) ((char *) &msg[core] - (char *) msg);*/
      off_t offset = 0;
      do {
        /*int s = e_read(mem, 0, 0, offset, &msg[core],
         * sizeof(msg[core]));*/
        int s = e_read(mem, 0, 0, offset, msg, E_CORES * sizeof(msg[core]));
        if (s == E_ERR) {
          log("error en `e_read`.");
          exit(1);
        }
        /*nano_wait(0, 1000);*/
        logf("[%u] %u %u %u %p %p\n",
             core,
             msg[core].status,
             msg[core].job,
             msg[core].param,
             (void *) msg[core].response.s,
             (void *) msg[core].response.v);
        logf("%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
             msg[0].status,
             msg[1].status,
             msg[2].status,
             msg[3].status,
             msg[4].status,
             msg[5].status,
             msg[6].status,
             msg[7].status,
             msg[8].status,
             msg[9].status,
             msg[10].status,
             msg[11].status,
             msg[12].status,
             msg[13].status,
             msg[14].status,
             msg[15].status);
      } while (msg[core].status == B_JOB_TO_DO);
      response[core] = msg[core].response.s;
    }
  }

  e_reset_group(device);

  return response;
}
