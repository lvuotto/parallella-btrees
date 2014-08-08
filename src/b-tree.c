
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
static b_node_t *  b_node_new         ();
static b_node_t *  b_node_add_nonfull (b_node_t **node, b_key_t key);
static void        b_node_replace     (b_node_t *node, b_key_t key, int i);
static int         b_node_index       (const b_node_t *node, b_key_t key);
static b_node_t ** b_node_find        (const b_tree_t *tree, b_key_t key);
static void        b_node_delete      (b_node_t *node);
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
  int i;
  
  n = b_node_find(tree, key);
  
  if (*n == NULL) {
    b_node_add_nonfull(n, key);
  } else {
    i = b_node_index(*n, key);
    
    if (key == (*n)->keys[i])
      return;
    
    if ((*n)->used_keys == B_MAX_KEYS)
      b_node_add_full(n, key);
    else
      b_node_add_nonfull(n, key);
  }
  
  if (tree->root->parent != NULL)
    tree->root = tree->root->parent;
}


bool b_find (const b_tree_t *tree, b_key_t key) {
  b_node_t **n;
  int i;
  
  n = b_node_find(tree, key);
  if (*n == NULL)
    return false;
  
  i = b_node_index(*n, key);
  
  return i < (*n)->used_keys && key == (*n)->keys[i];
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
    node->children[i] = NULL;
    node->keys[i] = B_INVALID_KEY;
  }
  node->children[B_MAX_KEYS] = NULL;
  node->parent = NULL;
  
  return node;
}


/**
 * Devuelve el nodo en el que se produjo la inserción.
 * Útil para obtener el nuevo padre en el momento en
 * que se realiza el split.
 * 
 * TODO:
 *  - Los padres no matchean.
 **/
B_EXPORT b_node_t * b_node_add_nonfull (b_node_t **node, b_key_t key) {
  int i, j;
  
  if (*node == NULL) {
    
    *node = b_node_new();
    (*node)->keys[0] = key;
    (*node)->used_keys = 1;
    
  } else {
    
    /* Tengo que insertar en la posición `i`, pues esta función es llamada
     * sii `key` no pertenece al árbol.
     * Se mueven los hijos adelante, salvo por el último caso, pues o bien
     * esta inserción se produce sobre una hoja, o bien se produce por un
     * split, y en ese caso, cuando se vuelva del llamado recursivo, los hijos
     * serán asignados correctamente. */
    i = b_node_index(*node, key);
    for (j = (*node)->used_keys; j > i; j--) {
      (*node)->keys[j] = (*node)->keys[j - 1];
      (*node)->children[j + 1] = (*node)->children[j];
    }
    (*node)->keys[i] = key;
    (*node)->used_keys++;
    
  }
  
  return *node;
}


/**
 * Devuelve el nodo en el que se produjo la inserción.
 * Útil para obtener el nuevo padre en el momento en
 * que se realiza el split.
 * 
 * TODO:
 *  - Los padres no matchean.
 **/
B_EXPORT b_node_t * b_node_add_full (b_node_t **node, b_key_t key) {
  b_node_t *n, *backup;
  int i, j, k;
  
  /* Hay que splitear :s */
  i = B_MAX_KEYS / 2;
  if (key < (*node)->keys[i - 1]) {
    i--;
    k = (*node)->keys[i];
    b_node_replace(*node, key, i);
  } else if (key < (*node)->keys[i]) {
    k = key;
  } else {
    k = (*node)->keys[i];
    b_node_replace(*node, key, i);
  }
  
  n = b_node_new();
  for (j = B_MAX_KEYS / 2; j < B_MAX_KEYS; j++) {
    n->children[j - B_MAX_KEYS / 2] = (*node)->children[j];
    n->keys[j - B_MAX_KEYS / 2] = (*node)->keys[j];
  }
  n->children[j - B_MAX_KEYS / 2] = (*node)->children[j];
  n->used_keys = B_MAX_KEYS - B_MAX_KEYS / 2;
  (*node)->used_keys = B_MAX_KEYS / 2;
  
  if (key > (*node)->keys[B_MAX_KEYS / 2 - 1]) {
    /* Evito arruinar la estructura al acomodar los padres al final, pues si
     * estoy en un llamado recursivo, al volver mi hijo se encargará de
     * acomodar a sus hermanos como se debe. */
    i = b_node_index(n, key);
    n->children[i] = NULL;
  }
  
  if ((*node)->parent == NULL || (*node)->parent->used_keys < B_MAX_KEYS) {
    n->parent = b_node_add_nonfull(& (*node)->parent, k);
    i = b_node_index(n->parent, k);
    n->parent->children[i] = *node;
    n->parent->children[i + 1] = n;
  } else {
    
    /**
     * El padre está hasta las manos. Hay que romperse la cabeza.
     * 
     * Casos:
     *  1. el valor a insertar en el padre es menor que el valor medio =>
     *     => `*node` y `n` tendrán como padre al padre de `*node`.
     *  2. el valor a insertar en el padre _es_ el valor medio =>
     *     => `*node` conservará a su padre y `n` tendrá como padre al nodo
     *     creado en el llamado recursivo.
     *  3. el valor a insertar en el padre es mayor que el valor medio =>
     *     => `*node` y `n` tendrán como padre al nodo creado en el llamado
     *     recursivo.
     **/
    
    if (k < (*node)->parent->keys[B_MAX_KEYS / 2 - 1]) {
      
      /* k < keys[m - 1] */
      b_node_add_full(& (*node)->parent, k);
      n->parent = (*node)->parent;
      i = b_node_index(n->parent, k);
      n->parent->children[i] = *node;
      n->parent->children[i + 1] = n;
      
    } else if (k < (*node)->parent->keys[B_MAX_KEYS / 2]) {
      
      /* keys[m - 1] < k < keys[m] */
      n->parent = b_node_add_full(& (*node)->parent, k);
      (*node)->parent->children[B_MAX_KEYS / 2] = *node;
      n->parent->children[0] = n;
      
    } else {
      
      /* keys[m] < k */
      n->parent = b_node_add_full(& (*node)->parent, k);
      (*node)->parent = n->parent;
      i = b_node_index(n->parent, k);
      n->parent->children[i] = *node;
      n->parent->children[i + 1] = n;
      
    }
    
  }
  
  backup = *node;
  for (j = 0; j <= n->used_keys; j++) {
    if (n->children[j] != NULL)
      n->children[j]->parent = n;
  }
  *node = backup;
  
  return n;
}


/**
 * `i` es la posición que quiero reemplazar con `key`.
 * 
 * TODO:
 *  - Fijarse si se debería hacer algo con los hijos. Como está ahora
 *    anda, pero andá a saber... -> hecho
 *  - Fijarse si se pierde algún hijo por el camino. -> resuelto en 
 *    `b_node_add_nonfull`.
 **/
B_EXPORT void b_node_replace (b_node_t *node, b_key_t key, int i) {
  int j;
  
  assert(0 <= i && i < node->used_keys);
  
  if (key < node->keys[i]) {
    for (j = i; j > 0 && key < node->keys[j]; j--) {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
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
B_EXPORT b_node_t ** b_node_find (const b_tree_t *tree, b_key_t key) {
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


/**
 * Libera recursivamente a los hijos, luego a si mismo, y se setea en NULL.
 * No se debe tocar al padre.
 **/
B_EXPORT void b_node_delete (b_node_t *node) {
  int i;
  
  if (node != NULL) {
    for (i = 0; i <= node->used_keys; i++) {
      b_node_delete(node->children[i]);
    }
    
    free(node);
    node = NULL;
  }
}
