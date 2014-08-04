
#ifndef __B_TREE_H__

#define __B_TREE_H__


#include <stdbool.h>


#ifndef B_MAX_KEYS
# define B_MAX_KEYS 2
#endif

#define B_INVALID_KEY (-404)

#ifdef B_DEBUG
# define B_EXPORT
# define B_EXPORT_INLINE
#else
# define B_EXPORT static
# define B_EXPORT_INLINE static inline
#endif


typedef int b_key_t;
typedef struct b_node_s b_node_t;
typedef struct b_tree_s b_tree_t;

struct b_node_s {
  b_key_t       keys[B_MAX_KEYS];
  int           used_keys;
  b_node_t     *childs[B_MAX_KEYS + 1];
  b_node_t     *parent;
};

struct b_tree_s {
  b_node_t     *root;
};


b_tree_t * b_new     ();
void       b_add     (b_tree_t *tree, b_key_t key);
bool       b_find    (const b_tree_t *tree, b_key_t key);
void       b_delete  (b_tree_t *tree);


#ifdef B_DEBUG

B_EXPORT b_node_t *  b_node_new     ();
B_EXPORT b_node_t *  b_node_add     (b_node_t **node, b_key_t key);
B_EXPORT void        b_node_replace (b_node_t *node, b_key_t key, int i);
B_EXPORT int         b_node_index   (const b_node_t *node, b_key_t key);
B_EXPORT b_node_t ** b_node_find    (const b_tree_t *tree, b_key_t key);
B_EXPORT void        b_node_delete  (b_node_t *node);

#endif


/**
 * 
 * Invariante del árbol:
 *  - 0 <= `used_keys` <= B_MAX_KEYS.
 *  - `keys` está ordenado.
 *  - `value` != NULL sii el nodo es una hoja.
 *  - keys[i] < childs[i+1]->keys[0] <= childs[i+1]->keys[used_keys] < keys[i+1]
 *    (si 0 <= i < used_keys)
 *  - childs[0]->keys[used_keys] < keys[0]
 *  - keys[used_keys] < childs[used_keys]->keys[0]
 * 
 * Observaciones:
 *  - El árbol puede contener basura o residuos de su estructura en estados
 *    anteriores. `used_keys` se utiliza para poner una frontera entre lo 
 *    que es útil y lo que es residuos.
 * 
 **/


#endif  /* __B_TREE_H__ */
