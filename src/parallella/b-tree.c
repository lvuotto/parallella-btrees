#include "b-tree.h"

#include <stdlib.h>
#include <assert.h>
#include "e-lib.h"


/* ==========================================================================
 * PROTOTIPOS
 * ========================================================================== */


static b_node_t *  b_node_new         ();
static b_node_t *  b_node_add_nonfull (b_node_t **node, b_key_t key);
static b_node_t *  b_node_add_full    (b_node_t **node, b_key_t key);
static void        b_node_replace     (b_node_t *node, b_key_t key, int i);
static int         b_node_index       (const b_node_t *node, b_key_t key);
static b_node_t ** b_node_find        (const b_tree_t *tree, b_key_t key);
static void        b_node_delete      (b_node_t *node);


/* ==========================================================================
 * FUNCIONES ÁRBOL
 * ========================================================================== */


b_tree_t * b_new()
{
  b_tree_t *tree = (b_tree_t *) malloc(sizeof(b_tree_t));
  tree->root = NULL;
  
  return tree;
}


void b_add(b_tree_t *tree, b_key_t key)
{
  b_node_t **n = b_node_find(tree, key);
  
  if (*n == NULL) {
    b_node_add_nonfull(n, key);
  } else {
    int i = b_node_index(*n, key);
    
    if (i < (*n)->used_keys && key == (*n)->keys[i])
      return;
    
    if ((*n)->used_keys == B_MAX_KEYS)
      b_node_add_full(n, key);
    else
      b_node_add_nonfull(n, key);
  }
  
  if (tree->root->parent != NULL)
    tree->root = tree->root->parent;
}


void b_add_parallel(b_tree_t *tree, b_key_t keys[], size_t n)
{
  
}


bool b_find(const b_tree_t *tree, b_key_t key)
{
  b_node_t **n = b_node_find(tree, key);
  if (*n == NULL)
    return false;
  
  int i = b_node_index(*n, key);
  return i < (*n)->used_keys && key == (*n)->keys[i];
}


void b_delete(b_tree_t *tree)
{
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


static b_node_t * b_node_new()
{
  b_node_t *node = (b_node_t *) malloc(sizeof(b_node_t));
  node->used_keys = 0;
  for (int i = 0; i < B_MAX_KEYS; i++) {
    node->children[i] = NULL;
    node->keys[i] = 0;
  }
  node->children[B_MAX_KEYS] = NULL;
  node->parent = NULL;
  
  return node;
}


/**
 * Devuelve el nodo en el que se produjo la inserción. Útil para obtener
 * el nuevo padre en el momento en que se realiza el split.
 **/
static b_node_t * b_node_add_nonfull(b_node_t **node, b_key_t key)
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
 * Devuelve el nodo en el que se produjo la inserción. Útil para obtener
 * el nuevo padre en el momento en que se realiza el split.
 **/
static b_node_t * b_node_add_full(b_node_t **node, b_key_t key)
{
  /**
   * Se guarda para el futuro, pues `b_node_replace` va a hacer que
   * el puntero de este hijo se pierda.
   **/
  b_node_t *m_child = (*node)->children[B_MAX_KEYS / 2];
  b_key_t up_key;
  if (key < (*node)->keys[B_MAX_KEYS / 2 - 1]) {
    up_key = (*node)->keys[B_MAX_KEYS / 2 - 1];
    b_node_replace(*node, key, B_MAX_KEYS / 2 - 1);
  } else if (key < (*node)->keys[B_MAX_KEYS / 2]) {
    up_key = key;
  } else {
    up_key = (*node)->keys[B_MAX_KEYS / 2];
    b_node_replace(*node, key, B_MAX_KEYS / 2);
  }
  
  b_node_t *new_child = b_node_new();
  int split;
  for (split = B_MAX_KEYS / 2; split < B_MAX_KEYS; split++) {
    new_child->children[split - B_MAX_KEYS / 2] = (*node)->children[split];
    new_child->keys[split - B_MAX_KEYS / 2] = (*node)->keys[split];
  }
  new_child->children[split - B_MAX_KEYS / 2] = (*node)->children[split];
  new_child->used_keys = B_MAX_KEYS - B_MAX_KEYS / 2;
  (*node)->used_keys = B_MAX_KEYS / 2;
  
  if (key > up_key) {
    (*node)->children[B_MAX_KEYS / 2] = m_child;
    /**
     * Evito arruinar la estructura al acomodar los padres al final, pues
     * este hijo es apuntado por `new_child` y a `*node`, y al modificar
     * a su padre el cambio repercute en `*node`, y necesito que este se 
     * mantenga en el caso en que esté en un llamado recursivo.
     **/
    int i = b_node_index(new_child, key);
    new_child->children[i] = NULL;
    new_child->children[i + 1] = NULL;
  } else {
    new_child->children[0] = m_child;
  }
  
  if ((*node)->parent == NULL || (*node)->parent->used_keys < B_MAX_KEYS) {
    new_child->parent = b_node_add_nonfull(& (*node)->parent, up_key);
    int i = b_node_index(new_child->parent, up_key);
    new_child->parent->children[i] = *node;
    new_child->parent->children[i + 1] = new_child;
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
    if (up_key < (*node)->parent->keys[B_MAX_KEYS / 2 - 1]) {
      /* k < keys[m - 1] */
      b_node_add_full(& (*node)->parent, up_key);
      new_child->parent = (*node)->parent;
      int i = b_node_index(new_child->parent, up_key);
      new_child->parent->children[i] = *node;
      new_child->parent->children[i + 1] = new_child;
    } else if (up_key < (*node)->parent->keys[B_MAX_KEYS / 2]) {
      /* keys[m - 1] < k < keys[m] */
      new_child->parent = b_node_add_full(& (*node)->parent, up_key);
      (*node)->parent->children[B_MAX_KEYS / 2] = *node;
      new_child->parent->children[0] = new_child;
    } else {
      /* keys[m] < k */
      new_child->parent = b_node_add_full(& (*node)->parent, up_key);
      (*node)->parent = new_child->parent;
      int i = b_node_index(new_child->parent, up_key);
      new_child->parent->children[i] = *node;
      new_child->parent->children[i + 1] = new_child;
    }
  }
  
  b_node_t *backup = *node;
  for (int i = 0; i <= new_child->used_keys; i++) {
    if (new_child->children[i] != NULL)
      new_child->children[i]->parent = new_child;
  }
  *node = backup;
  
  return new_child;
}


/**
 * Reemplaza la clave de `index` por `key`, reordenando las claves y los
 * hijos.
 **/
static void b_node_replace(b_node_t *node, b_key_t key, int index)
{
  assert(0 <= index && index < node->used_keys);
  
  int new;
  if (key < node->keys[index]) {
    for (new = index; new > 0 && key < node->keys[new - 1]; new--) {
      node->keys[new] = node->keys[new - 1];
      node->children[new + 1] = node->children[new];
    }
  } else {
    for (new = index;
         new < node->used_keys - 1 && key > node->keys[new + 1];
         new++)
    {
      node->keys[new] = node->keys[new + 1];
      node->children[new] = node->children[new + 1];
    }
  }
  node->keys[new] = key;
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
  assert(node != NULL);
  
  int i;
  for (i = 0; i < node->used_keys && key > node->keys[i]; i++) {}
  return i;
#else
  assert(node != NULL);
  
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
static b_node_t ** b_node_find(const b_tree_t *tree, b_key_t key)
{
  assert(tree != NULL);
  
  b_node_t **n = (b_node_t **) tree;
  while (*n != NULL) {
    int i = b_node_index(*n, key);
    if ((i == (*n)->used_keys || key != (*n)->keys[i]) &&
        (*n)->children[i] != NULL)
    {
      n = &(*n)->children[i];
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
static void b_node_delete(b_node_t *node)
{
  if (node != NULL) {
    for (int i = 0; i <= node->used_keys; i++) {
      b_node_delete(node->children[i]);
    }
    
    free(node);
    node = NULL;
  }
}
