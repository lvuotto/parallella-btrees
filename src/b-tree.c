
#include "b-tree.h"

#ifdef B_DEBUG
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


static b_node_t ** b_find_node   (b_tree_t *tree, b_key_t key);

static b_node_t *  b_node_new    ();
static void        b_node_delete (b_node_t *node);
static int         b_node_index  (b_node_t *node, b_key_t key);


/* ==========================================================================
 * FUNCIONES ÁRBOL
 * ========================================================================== */


b_tree_t * b_new () {
  b_tree_t *tree;
  
  tree = (b_tree_t *) malloc(sizeof(b_tree_t));
  tree->root = NULL;
  
  return tree;
}


void b_add (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i, j;
  
  n = b_find_node(tree, key);
  if (*n == NULL) {
    *n = b_node_new();
    (*n)->keys[0] = key;
    (*n)->used_keys++;
  } else {
    if ((*n)->used_keys == B_MAX_KEYS) {
      /* b_split(); */
    } else {
      i = b_node_index(*n, key);
      if (key != (*n)->keys[i]) {
        /* Tengo que insertar en la posición i + 1. Also, las inserciones
         * son en las hojas => childs[j] == NULL para todo j. */
        i++;
        for (j = (*n)->used_keys; j > i; j--) {
          (*n)->keys[j] = (*n)->keys[j - 1];
        }
        (*n)->keys[i] = key;
        (*n)->used_keys++;
      }
    }
  }
}


bool b_find (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  n = b_find_node(tree, key);
  if (*n == NULL)
    return false;
  
  i = b_node_index(*n, key);
  
  return key == (*n)->keys[i];
}


void b_delete (b_tree_t *tree) {
  assert(tree != NULL);
  
  if (tree->root != NULL) {
    b_node_delete(tree->root);
  }
  free(tree);
  tree = NULL;
}


/**
 * Devuelve un puntero al nodo* donde se encuentra `key`, o donde
 * habría que insertarlo.
 **/
static b_node_t ** b_find_node (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  assert(tree != NULL);
  
  n = (b_node_t **) tree;
  i = 0;
  while (*n != NULL && key != (*n)->keys[i]) {
    i = b_node_index(*n, key);
    if ((i == -1 || key != (*n)->keys[i]) && (*n)->childs[i + 1] != NULL) {
      n = &((*n)->childs[i + 1]);
    } else {
      /* match concreto o fin de la "recursión". */
      break;
    }
  }
  
  return n;
}


/* ==========================================================================
 * FUNCIONES NODOS
 * ========================================================================== */


static b_node_t * b_node_new () {
  b_node_t *node;
  int i;
  
  node = (b_node_t *) malloc(sizeof(b_node_t));
  node->used_keys = 0;
  for (i = 0; i < B_MAX_KEYS + 1; i++) {
    node->childs[i] = NULL;
  }
  
  return node;
}


/**
 * 
 * Devuelve -1 si `key` es menor a la menor de las claves, y un número
 * entre 0 y node->used_keys (no inclusive) en caso contrario. El núme-
 * ro devuelto es o bien el índice donde se encuentra `key`, en caso de
 * que sea una de las claves del nodo, o bien es el índice en el cual se
 * encuentra la clave tal que node->keys[i] < `key` < node->keys[i+1].
 * 
 * TODO:
 *  - Fijarse si mejora con una busqueda lineal. O sea, son 16 claves.
 *    El caché debería ser mágico acá.
 **/
static int b_node_index (b_node_t *node, b_key_t key) {

#ifdef B_LINEAR_SEARCH
  int i;
  
  if (node->used_keys == 0 || key < node->keys[0])
    return -1;
  
  for (i = 0; i < node->used_keys && keys > node->keys[i]; i++);
  
  return i;
#else
  int l, u, p;
  
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


static void b_node_delete (b_node_t *node) {
  int i;
  
  for (i = 0; i < B_MAX_KEYS + 1; i++) {
    if (node->childs[i] != NULL) {
#ifdef B_DEBUG
      fprintf(stderr, "Eliminando %d\n", i);
#endif
      b_node_delete(node->childs[i]);
    }
  }
  
  free(node);
  node = NULL;
}
