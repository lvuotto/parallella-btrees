
#include <stdio.h>
#include <assert.h>
#include "b-tree.h"


void print_node (b_node_t *node, int t) {
  int i, j;
  char tab[256];
  
  for (j = 0; j < t; j++) {
    tab[j] = ' ';
  }
  tab[t] = 0;
  if (node != NULL) {
    printf("%s{", tab);
    if (node->childs[0] == NULL) {
      printf("}\n");
    } else {
      printf("\n");
      print_node(node->childs[0], t+2);
      printf("%s}\n", tab);
    }
    for (i = 0; i < node->used_keys; i++) {
      printf("%s'%d' [%d]\n%s{", tab, node->keys[i], node->used_keys, tab);
      if (node->childs[i] == NULL) {
        printf("}\n");
      } else {
        printf("\n");
        print_node(node->childs[i+1], t+2);
        printf("%s}\n", tab);
      }
    }
  } else {
    printf("%snil\n", tab);
  }
}


int main () {
  
  b_tree_t *tree;
  int i;
  
  tree = b_new();
  
  for (i = 1; i <= 7; i++) {
    b_add(tree, i);
  }
  
  print_node(tree->root, 0);
  
  b_delete(tree);
  
  return 0;
  
}
