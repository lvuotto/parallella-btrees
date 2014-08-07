
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
 * 
 * TODO:
 *  - Los padres no matchean.
 **/
B_EXPORT b_node_t * b_node_add (b_node_t **node, b_key_t key) {
  b_node_t *n, *r, *c;
  int i, j, k;
  
  if (*node == NULL) {
    
    *node = b_node_new();
    (*node)->keys[0] = key;
    (*node)->used_keys++;
    r = *node;
    
  } else if ((*node)->used_keys < B_MAX_KEYS) {
    
    /* Tengo que insertar en la posición `i`, pues esta función es llamada
     * sii `key` no pertenece al árbol.
     * Se mueven los hijos adelante, salvo por el último caso, pues o bien
     * esta inserción se produce sobre una hoja, o bien se produce por un
     * split, y en ese caso, cuando se vuelva del llamado recursivo, los hijos
     * serán asignados correctamente. */
    i = b_node_index(*node, key);
    for (j = (*node)->used_keys; j > i; j--) {
      (*node)->keys[j] = (*node)->keys[j - 1];
      (*node)->childs[j + 1] = (*node)->childs[j];
    }
    (*node)->keys[i] = key;
    (*node)->used_keys++;
    r = *node;
    
  } else {
    
    /* Hay que splitear :s */
    i = B_MAX_KEYS / 2;
    c = (*node)->childs[i];
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
      n->childs[j - B_MAX_KEYS / 2] = (*node)->childs[j];
      n->keys[j - B_MAX_KEYS / 2] = (*node)->keys[j];
    }
    n->childs[j - B_MAX_KEYS / 2] = (*node)->childs[j];
    n->used_keys = B_MAX_KEYS - B_MAX_KEYS / 2;
    (*node)->used_keys = B_MAX_KEYS / 2;
    
    if (key < k) {
      n->childs[0] = c;
    } else {
      /* si k == key no hay drama, pues `c` será reemplazado luego. */
      (*node)->childs[B_MAX_KEYS / 2] = c;
    }
    
    if ((*node)->parent == NULL || (*node)->parent->used_keys < B_MAX_KEYS) {
      n->parent = b_node_add(&(*node)->parent, k);
      
      assert(n->parent == (*node)->parent);
      
      i = b_node_index(n->parent, k);
      n->parent->childs[i] = *node;
      n->parent->childs[i + 1] = n;
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
        b_node_add(&((*node)->parent), k);
        n->parent = (*node)->parent;
        i = b_node_index(n->parent, k);
        n->parent->childs[i] = *node;
        n->parent->childs[i + 1] = n;
        
      } else if (k < (*node)->parent->keys[B_MAX_KEYS / 2]) {
        
        /* keys[m - 1] < k < keys[m] */
        n->parent = b_node_add(&((*node)->parent), k);
        (*node)->parent->childs[B_MAX_KEYS / 2] = *node;
        n->parent->childs[0] = n;
        
      } else {
        
        /* keys[m] < k */
        n->parent = b_node_add(&((*node)->parent), k);
        (*node)->parent = n->parent;
        i = b_node_index(n->parent, k);
        n->parent->childs[i] = *node;
        n->parent->childs[i + 1] = n;
        
      }
      
    }
    
    r = n;
  }
  
  return r;
}


/**
 * `i` es la posición que quiero reemplazar con `key`.
 * 
 * TODO:
 *  - Fijarse si se debería hacer algo con los hijos. Como está ahora
 *    anda, pero andá a saber... -> hecho
 *  - Fijarse si se pierde algún hijo por el camino. -> resuelto en 
 *    `b_node_add`.
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
        (*n)->childs[i] != NULL)
    {
      n = &((*n)->childs[i]);
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
      b_node_delete(node->childs[i]);
    }
    
    free(node);
    node = NULL;
  }
}
