
#include <stdio.h>
#include <assert.h>
#include "b-tree.h"


int main () {
  
  b_tree_t *tree;
  int i;
  
  tree = b_new();
  
  tree->root = b_node_new();
  tree->root->childs[0] = b_node_new();
  for (i = 0; i < B_MAX_KEYS; i++) {
    tree->root->keys[i] = 3*i + 3;
    tree->root->childs[i+1] = b_node_new();
  }
  
  b_delete(tree);
  
  return 0;
  
}
