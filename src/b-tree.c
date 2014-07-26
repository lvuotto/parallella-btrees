
#include "b-tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


static b_node_t * b_node_new    ();
static void       b_node_delete (b_node_t *node);
static int        b_node_index  (b_node_t *node, b_key_t key);

static inline void b_swap (void *a, void *b);


/* ==========================================================================
 * FUNCIONES ÁRBOL
 * ========================================================================== */


b_tree_t * b_new () {
  b_tree_t *tree;
  
  tree = (b_tree_t *) malloc(sizeof(b_tree_t));
  tree->root = NULL;
  
  return tree;
}


/**
 * `value` guarda una copia de val.
 **/
void b_add (b_tree_t *tree, b_key_t key, void *val) {
  b_node_t **n;
  
  if (tree->root == NULL) {
    tree->root = b_node_new();
  } else {
    n = b_find(tree, key);
    if (*n == NULL) {
      *n = b_node_new();
    } else {
      free((*n)->value);
    }
    
    (*n)->value = val;
  }
}


void b_add_key (b_tree_t *tree, b_key_t key) {
  b_add(tree, key, NULL);
}


/**
 * Devuelve el nodo donde se encuentra `key`, o NULL en caso contrario.
 **/
b_node_t ** b_find (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  static int i;
  
  n = (b_node_t **) tree;
  i = 0;
  while (*n != NULL || key != (*n)->keys[i]) {
    i = b_node_index(*n, key);
    if (i == -1 || key != (*n)->keys[i]) {
      n = &((*n)->childs[i + 1]);
    } else {
      /* match concreto */
      break;
    }
  }
  
  return n;
}


void b_delete (b_tree_t *tree) {
  if (tree->root != NULL) {
    b_node_delete(tree->root);
  }
  free(tree);
  tree = NULL;
}


/* ==========================================================================
 * FUNCIONES NODOS
 * ========================================================================== */


static b_node_t * b_node_new () {
  b_node_t *node;
  static int i;
  
  node = (b_node_t *) malloc(sizeof(b_node_t));
  node->used_keys = 0;
  for (i = 0; i < B_MAX_KEYS + 1; i++) {
    node->childs[i] = NULL;
  }
  node->value = NULL;
  
  return node;
}


/**
 * TODO:
 *  - Fijarse si mejora con una busqueda lineal. O sea, son 16 claves.
 *    El caché debería ser mágico acá.
 **/
static int b_node_index (b_node_t *node, b_key_t key) {
  static int l, u, p;
  
  if (key < node->keys[0])
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
}


static void b_node_delete (b_node_t *node) {
  int i;
  
  for (i = 0; i < B_MAX_KEYS + 1; i++) {
    if (node->childs[i] != NULL) {
      printf("Eliminando %d\n", i);
      b_node_delete(node->childs[i]);
    }
  }
  
  free(node->value);
  free(node);
  node = NULL;
}


/* ==========================================================================
 * FUNCIONES AUXILIARES
 * ========================================================================== */


static inline void b_swap (void *a, void *b) {
  static void *t;
  t = a;
  a = b;
  b = t;
}
