
#include <stdio.h>
#include <assert.h>
#include "b-tree.h"


void print_node (b_node_t *node) {
  int i;
  
  printf("%p\n", (void *) node);
  for (i = 0; i < node->used_keys; i++) {
    printf("  node->keys[%d] = %d\n", i, node->keys[i]);
    printf("  node->childs[%d] = %p\n", i, (void *) node->childs[i]);
  }
}


int main () {
  
  b_tree_t *tree;
  
  tree = b_new();
  
  b_add(tree, 3);
  b_add(tree, 6);
  b_add(tree, 9);
  
  assert(b_find(tree, 3));
  assert(b_find(tree, 6));
  assert(b_find(tree, 9));
  assert(!b_find(tree, 5));
  
  print_node(tree->root);
  
  b_delete(tree);
  
  return 0;
  
}
