
#ifndef __B_TREE_H__

#define __B_TREE_H__


#include <stdbool.h>


#ifndef B_MAX_KEYS
# define B_MAX_KEYS 16
#endif


typedef int b_key_t;
typedef struct b_node_s b_node_t;
typedef struct b_tree_s b_tree_t;

struct b_node_s {
  b_key_t       keys[B_MAX_KEYS];
  int           used_keys;
  b_node_t     *children[B_MAX_KEYS + 1];
  b_node_t     *parent;
};

struct b_tree_s {
  b_node_t     *root;
};


b_tree_t * b_new     ();
void       b_add     (b_tree_t *tree, b_key_t key);
bool       b_find    (const b_tree_t *tree, b_key_t key);
void       b_delete  (b_tree_t *tree);


/**
 * 
 * Invariante de cada nodo árbol:
 *  - 0 <= `used_keys` <= B_MAX_KEYS.
 *  - `keys` está ordenado.
 *  - keys[i] < children[i+1]->keys[0] <=
 *    children[i+1]->keys[children[i+1]->used_keys - 1] < keys[i+1]
 *    (si 0 <= i < used_keys - 1)
 *  - max_key(children[0]) < keys[0]
 *  - keys[used_keys - 1] < min_key(children[used_keys])
 * 
 * Observaciones:
 *  - El árbol puede contener basura o residuos de su estructura en estados
 *    anteriores. `used_keys` se utiliza para poner una frontera entre lo 
 *    que es útil y lo que es residuos.
 * 
 **/


#endif  /* __B_TREE_H__ */
