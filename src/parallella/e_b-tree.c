#include "e_b-tree.h"

#include "e-lib.h"
#include <stddef.h>
#include "btmi.h"

#define BTMI_ADDRESS 0x8f000000
#define B_TREE       0x8f001000


/* ======================================================================
 * PROTOTIPOS
 * ====================================================================== */


static b_status_t b_node_add_nonfull(b_node_t **node, b_key_t key);
static int b_node_index(const b_node_t *node, b_key_t key);
static b_node_t ** b_node_find(const b_tree_t *tree,
                               b_key_t key,
                               volatile b_msg_t *s);


int main()
{
  unsigned int row, col;
  e_coords_from_coreid(e_get_coreid(), &row, &col);
  unsigned int core = row*e_group_config.group_cols + col;
  
  volatile b_msg_t *msg = (b_msg_t *) BTMI_ADDRESS;
  
  /* Implementar "servidor". */
  while (E_TRUE) {
    if (msg[core].status == B_JOB_TO_DO) {
      switch (msg[core].job) {
      case B_INSERT:
        msg[core].response.s = b_node_add_nonfull((b_node_t **) B_TREE,
                                                  msg[core].param);
        msg[core].response.v = 0;
        break;
      case B_FIND:
        b_node_find((b_tree_t *) B_TREE,
                    msg[core].param,
                    &msg[core]);
        break;
      default:
        msg[core].response.s = B_UNRECOGNIZED;
        msg[core].response.v = 0;
        break;
      }
      msg[core].status = B_STAND_BY;
      break;
    }
  }

  /*msg[core].status     += 100;
  msg[core].job        += 100;
  msg[core].param      += 100;
  msg[core].response.s += 100;
  msg[core].response.v += 100;*/

  /**
   * TODO:
   * - Implementar con interrupciones y hacer mediciones.
   **/

  return 0;
}


/* ======================================================================
 * FUNCIONES NODOS PARALELOS
 * ====================================================================== */


/* DUMMY */
static b_node_t * b_node_new() { return NULL; }


/**
 * Devuelve el nodo en el que se produjo la inserción. Útil para obtener
 * el nuevo padre en el momento en que se realiza el split.
 **/
static b_status_t b_node_add_nonfull(b_node_t **node, b_key_t key)
{
  if (*node == NULL) {
    *node = b_node_new();
    (*node)->keys[0] = key;
    (*node)->used_keys = 1;
  } else {
    /**
     * Tengo que insertar en la posición `i`, pues esta función es llamada
     * sii `key` no pertenece al árbol.
     * Se mueven los hijos adelante, salvo por el último caso, pues o bien
     * esta inserción se produce sobre una hoja, o bien se produce por un
     * split, y en ese caso, cuando se vuelva del llamado recursivo, los
     * hijos serán asignados correctamente.
     **/
    int i = b_node_index(*node, key);
    for (int j = (*node)->used_keys; j > i; j--) {
      (*node)->keys[j] = (*node)->keys[j - 1];
      (*node)->children[j + 1] = (*node)->children[j];
    }
    (*node)->keys[i] = key;
    (*node)->used_keys++;
  }
  
  /*return *node;*/
  return E_OK;
}


/**
 * TODO:
 *  - Fijarse si mejora con una busqueda lineal. O sea, con pocas claves,
 *    el caché debería ser mágico acá.
 *  - Hacer que la busqueda binaria funcione devuelve el índice de la máxima
 *    clave menor a `key` en caso de que esta no pertenezca al nodo.
 **/
static int b_node_index(const b_node_t *node, b_key_t key)
{
#ifndef B_BINARY_SEARCH
  int i;
  for (i = 0; i < node->used_keys && key > node->keys[i]; i++) {}
  return i;
#else
  if (node->used_keys == 0 || key < node->keys[0])
    return -1;
  
  int l, u, p;
  l = 0;
  u = node->used_keys - 1;
  while (l <= u) {
    p = (l + u)/2;
    if (key < node->keys[p]) {
      u = p - 1;
    } else if (key == node->keys[p]) {
      u = p;
      break;
    } else {
      l = p + 1;
    }
  }
  
  return u;
#endif
}


/**
 * Devuelve un puntero al nodo* donde se encuentra `key`, o donde
 * habría que insertarlo.
 **/
static b_node_t ** b_node_find(const b_tree_t *tree,
                               b_key_t key,
                               volatile b_msg_t *m)
{
  b_node_t **n = (b_node_t **) tree;
  while (*n != NULL) {
    int i = b_node_index(*n, key);
    if ((i == (*n)->used_keys || key != (*n)->keys[i]) &&
        (*n)->children[i] != NULL)
    {
      n = &(*n)->children[i];
    } else {
      /* match concreto ó fin de la "recursión". */
      m->response.v = (unsigned int) (*n)->used_keys; 
      m->response.s = i < (*n)->used_keys && key == (*n)->keys[i] ?
                      (unsigned int) *n :
                      0xdead;
      break;
    }
  }
  
  return n;
}
