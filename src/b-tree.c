
#include "b-tree.h"

#ifdef B_DEBUG
# include <stdio.h>
#endif

#include <stdlib.h>
#include <assert.h>


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


static b_node_t *  b_node_new     ();
static void        b_node_add     (b_node_t **node, b_key_t key);
static void        b_node_replace (b_node_t *node, b_key_t key, int i);
static int         b_node_index   (b_node_t *node, b_key_t key);
static b_node_t ** b_node_find    (b_tree_t *tree, b_key_t key);
static void        b_node_delete  (b_node_t *node);

static inline void b_swap        (void *a, void *b);


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
  b_node_t **n, *root;
  
  n = b_node_find(tree, key);
  b_node_add(n, key);
  
  root = *n;
  while (root->parent != NULL) {
    root = root->parent;
  }
  tree->root = root;
}


bool b_find (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  n = b_node_find(tree, key);
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
  node->parent = NULL;
  
  return node;
}


static void b_node_add (b_node_t **node, b_key_t key) {
  b_node_t *n;
  int i, j, k;
  
  if (*node == NULL) {
    *node = b_node_new();
    (*node)->keys[0] = key;
    (*node)->used_keys++;
  } else if ((*node)->used_keys < B_MAX_KEYS) {
    i = b_node_index(*node, key);
    if (key != (*node)->keys[i]) {
      /* Tengo que insertar en la posición i + 1. Also, las inserciones
       * son en las hojas => childs[j] == NULL para todo j. */
      i++;
      for (j = (*node)->used_keys; j > i; j--) {
        (*node)->keys[j] = (*node)->keys[j - 1];
      }
      (*node)->keys[i] = key;
      (*node)->used_keys++;
    }
  } else {
    i = B_MAX_KEYS/2;
    if (key < (*node)->keys[i])
      i--;
    
    k = (*node)->keys[i];
    b_node_replace(*node, key, i);
    
    n = b_node_new();
    for (j = 0; j < i; j++) {
      n->keys[j] = (*node)->keys[j + i];
      n->childs[j] = (*node)->childs[j + i];
    }
    n->used_keys = i;
    (*node)->used_keys = i;
    
    b_node_add(&(*node)->parent, k);
    n->parent = (*node)->parent;
    i = b_node_index((*node)->parent, k);
    (*node)->parent->childs[i] = *node;
    (*node)->parent->childs[i + 1] = n;
  }
}


/**
 * `i` es la posición que quiero reemplazar con `key`
 **/
static void b_node_replace (b_node_t *node, b_key_t key, int i) {
  int j, k;
  
  k = node->keys[i];
  node->keys[i] = key; 
  if (key < k) {
    for (j = i; j > 0 && node->keys[j] < node->keys[j - 1]; j--) {
      b_swap(&node->keys[j], &node->keys[j - 1]);
      b_swap(node->childs[j], node->childs[j - 1]);
    }
  } else {
    for (j = i + 1; j < B_MAX_KEYS && node->keys[j - 1] > node->keys[j]; j++) {
      b_swap(&node->keys[j - 1], &node->keys[j]);
      b_swap(node->childs[j - 1], node->childs[j]);
    }
  }
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


/**
 * Devuelve un puntero al nodo* donde se encuentra `key`, o donde
 * habría que insertarlo.
 **/
static b_node_t ** b_node_find (b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  assert(tree != NULL);
  
  n = (b_node_t **) tree;
  while (*n != NULL) {
    i = b_node_index(*n, key);
    if ((i == -1 || key != (*n)->keys[i]) && (*n)->childs[i + 1] != NULL) {
      n = &((*n)->childs[i + 1]);
    } else {
      /* match concreto ó fin de la "recursión". */
      break;
    }
  }
  
  return n;
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


/* ==========================================================================
 * FUNCIONES AUXILIARES
 * ========================================================================== */


static inline void b_swap (void *a, void *b) {
  static void *t;
  
  t = a;
  a = b;
  b = t;
}
