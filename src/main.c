
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "b-tree.h"


#define HEAVY_TEST 8


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
      printf("%s'%d' [%d] [%p] [%p]\n%s{",
             tab,
             node->keys[i],
             node->used_keys,
             (void *) node,
             (void *) node->parent,
             tab);
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
  /*int a[HEAVY_TEST];
  int i, j, m;
  
  srand(176);
  
  tree = b_new();
  
  for (i = 0; i < HEAVY_TEST; i++) {
    a[i] = i;
  }
  j = HEAVY_TEST;
  for (i = 0; i < HEAVY_TEST; i++) {
    j = i + (rand() % (HEAVY_TEST - i));
    m = a[j];
    a[j] = a[i];
    a[i] = m;
  }
  
  for (i = 0; i < HEAVY_TEST; i++) {
    b_add(tree, a[i]);
  }
  
  print_node(tree->root, 0);
  
  for (i = 0; i < HEAVY_TEST; i++) {
    m = b_find(tree, a[i]);
    printf("[%d] -> %s\n", a[i], m ? "ok" : "no esta :(");
    if (!m) break;
  }
  
  printf("%s\n", m ? "estan todos!" : "fallo :'(");
  
  b_delete(tree);*/
  
  tree = b_new();
  b_add(tree, 1);
  b_add(tree, 2);
  b_add(tree, 3);
  b_add(tree, 6);
  b_add(tree, 7);
  b_add(tree, 5);
  print_node(tree->root, 0);
  puts("");
  b_add(tree, 4);
  print_node(tree->root, 0);
  b_delete(tree);
  
  return 0;
  
}
