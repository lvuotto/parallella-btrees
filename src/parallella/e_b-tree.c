
#include "e-lib.h"
#include <stdlib.h>

#include "btmi.h"


#define BTMI_ADDRESS 0x8f000000


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


static b_node_t *  b_node_add_nonfull (b_node_t **node, b_key_t key);
static void        b_node_replace     (b_node_t *node, b_key_t key, int i);
static int         b_node_index       (const b_node_t *node, b_key_t key);
static b_node_t ** b_node_find        (const b_tree_t *tree, b_key_t key);


int main () {
  e_coreid_t coreid = e_get_coreid();
  unsigned int row, col, core;

  e_coords_from_coreid(coreid, &row, &col);
  core = row*e_group_config.group_cols + col;

  volatile b_tree_msg_t *msg = (b_tree_msg_t *) BTMI_ADDRESS;
  while (E_TRUE) {
    
  }

  return 0;
}


/* ==========================================================================
 * FUNCIONES NODOS PARALELOS
 * ========================================================================== */


/**
 * Devuelve el nodo en el que se produjo la inserción. Útil para obtener
 * el nuevo padre en el momento en que se realiza el split.
 **/
static b_node_t * b_node_add_nonfull (b_node_t **node, b_key_t key) {
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
     * split, y en ese caso, cuando se vuelva del llamado recursivo, los hijos
     * serán asignados correctamente.
     **/
    int i = b_node_index(*node, key);
    for (int j = (*node)->used_keys; j > i; j--) {
      (*node)->keys[j] = (*node)->keys[j - 1];
      (*node)->children[j + 1] = (*node)->children[j];
    }
    (*node)->keys[i] = key;
    (*node)->used_keys++;
    
  }
  
  return *node;
}


/**
 * Reemplaza la clave de `index` por `key`, reordenando las claves y los
 * hijos.
 **/
static void b_node_replace (b_node_t *node, b_key_t key, int index) {
  int j;
  
  assert(0 <= index && index < node->used_keys);
  
  if (key < node->keys[index]) {
    for (j = index; j > 0 && key < node->keys[j - 1]; j--) {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
    }
  } else {
    for (j = index; j < node->used_keys - 1 && key > node->keys[j + 1]; j++) {
      node->keys[j] = node->keys[j + 1];
      node->children[j] = node->children[j + 1];
    }
  }
  
  node->keys[j] = key;
}


/**
 * TODO:
 *  - Fijarse si mejora con una busqueda lineal. O sea, con pocas claves,
 *    el caché debería ser mágico acá.
 *  - Hacer que la busqueda binaria funcione devuelve el índice de la máxima
 *    clave menor a `key` en caso de que esta no pertenezca al nodo.
 **/
static int b_node_index (const b_node_t *node, b_key_t key) {
#ifndef B_BINARY_SEARCH
  
  int i;
  
  assert(node != NULL);
  
  /*if (node->used_keys == 0 || key < node->keys[0])
    return -1;*/
  
  for (i = 0; i < node->used_keys && key > node->keys[i]; i++);
  
  return i;
  
#else

  int l, u, p;
  
  assert(node != NULL);
  
  if (node->used_keys == 0 || key < node->keys[0])
    return -1;
  
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
static b_node_t ** b_node_find (const b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  assert(tree != NULL);
  
  n = (b_node_t **) tree;
  while (*n != NULL) {
    i = b_node_index(*n, key);
    if ((i == (*n)->used_keys || key != (*n)->keys[i]) &&
        (*n)->children[i] != NULL)
    {
      n = & (*n)->children[i];
    } else {
      /* match concreto ó fin de la "recursión". */
      break;
    }
  }
  
  return n;
}

