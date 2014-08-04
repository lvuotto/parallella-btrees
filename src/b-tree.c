
#include "b-tree.h"

#ifdef B_DEBUG
# include <stdio.h>
#endif

#include <stdlib.h>
#include <assert.h>


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


#ifndef B_DEBUG
static b_node_t *  b_node_new     ();
static b_node_t *  b_node_add     (b_node_t **node, b_key_t key);
static void        b_node_replace (b_node_t *node, b_key_t key, int i);
static int         b_node_index   (const b_node_t *node, b_key_t key);
static b_node_t ** b_node_find    (const b_tree_t *tree, b_key_t key);
static void        b_node_delete  (b_node_t *node);
#endif


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
  
  n = b_node_find(tree, key);
  
  if (*n == NULL || key != (*n)->keys[b_node_index(*n, key)]) {
    b_node_add(n, key);
    
    if (tree->root->parent != NULL) {
      tree->root = tree->root->parent;
    }
  }
}


bool b_find (const b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  n = b_node_find(tree, key);
  if (*n == NULL)
    return false;
  
  i = b_node_index(*n, key);
  
  printf("%d == %d?\n", key, (*n)->keys[i]);
  
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


B_EXPORT b_node_t * b_node_new () {
  b_node_t *node;
  int i;
  
  node = (b_node_t *) malloc(sizeof(b_node_t));
  node->used_keys = 0;
  for (i = 0; i < B_MAX_KEYS; i++) {
    node->childs[i] = NULL;
    node->keys[i] = B_INVALID_KEY;
  }
  node->childs[B_MAX_KEYS] = NULL;
  node->parent = NULL;
  
  return node;
}


/**
 * Devuelve el nodo en el que se produjo la inserción.
 * Útil para obtener el nuevo padre en el momento en
 * que se realiza el split.
 **/
B_EXPORT b_node_t * b_node_add (b_node_t **node, b_key_t key) {
  b_node_t *n, *r;
  int i, j, k;
  
  if (*node == NULL) {
    
    *node = b_node_new();
    (*node)->keys[0] = key;
    (*node)->used_keys++;
    r = *node;
    
  } else if ((*node)->used_keys < B_MAX_KEYS) {
    
    /* Tengo que insertar en la posición i + 1, pues esta función es llamada
     * sii `key` no pertenece al árbol.
     * Se mueven los hijos adelante, salvo por el último caso, pues o bien
     * esta inserción se produce sobre una hoja, o bien se produce por un
     * split, y en ese caso, cuando se vuelva del llamado recursivo, los hijos
     * serán asignados correctamente. */
    i = b_node_index(*node, key) + 1;
    for (j = (*node)->used_keys; j > i; j--) {
      (*node)->keys[j] = (*node)->keys[j - 1];
      (*node)->childs[j + 1] = (*node)->childs[j];
    }
    (*node)->keys[i] = key;
    (*node)->used_keys++;
    r = *node;
    
  } else {
    
    /* Hay que splitear :s */
    b_node_split(node, key);
    
  }
  
  return r;
}


/**
 * `i` es la posición que quiero reemplazar con `key`.
 * 
 * TODO:
 *  - Fijarse si se debería hacer algo con los hijos. Como está ahora
 *    anda, pero andá a saber... -> hecho
 *  - Fijarse si se pierde algún hijo por el camino.
 **/
B_EXPORT void b_node_replace (b_node_t *node, b_key_t key, int i) {
  int j;
  
  assert(0 <= i && i < node->used_keys);
  
  if (key < node->keys[i]) {
    for (j = i; j > 0 && key < node->keys[j]; j--) {
      node->keys[j] = node->keys[j - 1];
      node->childs[j + 1] = node->childs[j];
    }
  } else {
    for (j = i; j < node->used_keys - 1 && key > node->keys[j + 1]; j++) {
      node->keys[j] = node->keys[j + 1];
      node->keys[j] = node->keys[j + 1];
    }
  }
  
  node->keys[j] = key;
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
 *  - Fijarse si mejora con una busqueda lineal. O sea, con pocas claves,
 *    el caché debería ser mágico acá.
 **/
B_EXPORT int b_node_index (const b_node_t *node, b_key_t key) {
#ifdef B_LINEAR_SEARCH
  
  int i;
  
  assert(node != NULL);
  
  if (node->used_keys == 0 || key < node->keys[0])
    return -1;
  
  for (i = 0; i < node->used_keys && keys > node->keys[i]; i++);
  
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
B_EXPORT b_node_t ** b_node_find (const b_tree_t *tree, b_key_t key) {
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


/**
 * Libera recursivamente a los hijos, luego a si mismo, y se setea en NULL.
 * No se debe tocar al padre.
 **/
B_EXPORT void b_node_delete (b_node_t *node) {
  int i;
  
  assert(node != NULL);
  
  for (i = 0; i <= B_MAX_KEYS; i++) {
    if (node->childs[i] != NULL) {
      b_node_delete(node->childs[i]);
    }
  }
  
  free(node);
  node = NULL;
}
