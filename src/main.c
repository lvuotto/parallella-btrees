
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
  /*b_node_t *node;*/
  int i, j;
  
  tree = b_new();
  
  for (i = 1; i <= 7; i++) {
    b_add(tree, i);
    print_node(tree->root, 0);
    getchar();
  }
  
  print_node(tree->root, 0);
  
  assert(b_find(tree, 7));
  assert(b_find(tree, 6));
  assert(b_find(tree, 5));
  assert(b_find(tree, 4));
  assert(b_find(tree, 3));
  assert(b_find(tree, 2));
  assert(b_find(tree, 1));

  b_delete(tree);
  
  /*node = b_node_new();

  node->keys[0] = 1;
  node->keys[1] = 3;
  node->keys[2] = 4;
  node->keys[3] = 5;
  node->keys[4] = 6;
  node->keys[5] = 7;
  node->keys[6] = 8;
  node->keys[7] = 9;
  node->used_keys = B_MAX_KEYS;
  node->childs[0] = (b_node_t *) 0x7;
  node->childs[1] = (b_node_t *) 0x8;
  node->childs[2] = (b_node_t *) 0x9;
  node->childs[3] = (b_node_t *) 0xa;
  node->childs[4] = (b_node_t *) 0xb;
  node->childs[5] = (b_node_t *) 0xc;
  node->childs[6] = (b_node_t *) 0xd;
  node->childs[7] = (b_node_t *) 0xe;
  node->childs[8] = (b_node_t *) 0xf;
  
  printf("[%p]", (void *) node->childs[0]);
  for (i = 0; i < B_MAX_KEYS; i++) {
    printf(" %d [%p]", node->keys[i], (void *) node->childs[i + 1]);
  }
  printf("\n");

  b_node_replace(node, 0, 6);
  printf("[%p]", (void *) node->childs[0]);
  for (i = 0; i < B_MAX_KEYS; i++) {
    printf(" %d [%p]", node->keys[i], (void *) node->childs[i + 1]);
  }
  printf("\n");
  
  b_node_replace(node, 8, 1);
  printf("[%p]", (void *) node->childs[0]);
  for (i = 0; i < B_MAX_KEYS; i++) {
    printf(" %d [%p]", node->keys[i], (void *) node->childs[i + 1]);
  }
  printf("\n");*/
  
  i = 10; j = 20;
  printf("%d, %d\n", i, j);
  b_swap_keys(&i, &j);
  printf("%d, %d\n", i, j);

  return 0;
  
}
