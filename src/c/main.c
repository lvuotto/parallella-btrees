#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "b-tree.h"


#define TOTAL_KEYS (1 << 22)
#define TAB_SIZE 4


void shuffle(int *a, size_t n)
{
  int t;
  for (size_t i = 0, j; i < n; i++) {
    j = i + (rand() % (n - i));
    t = a[j];
    a[j] = a[i];
    a[i] = t;
  }
}


void print_node(b_node_t *node, int t)
{
  char tab[256];
  
  for (int j = 0; j < t; j++) {
    tab[j] = ' ';
  }
  tab[t] = 0;
  
  if (node != NULL) {
    printf("%s{", tab);
    if (node->children[0] == NULL) {
      printf("}\n");
    } else {
      printf("\n");
      print_node(node->children[0], t + TAB_SIZE);
      printf("%s}\n", tab);
    }
    for (int i = 0; i < node->used_keys; i++) {
      printf("%s'%d' [%d] [n %p] [p %p]\n%s{",
             tab,
             node->keys[i],
             node->used_keys,
             (void *) node,
             (void *) node->parent,
             tab);
      if (node->children[i] == NULL) {
        printf("}\n");
      } else {
        printf("\n");
        print_node(node->children[i + 1], t + TAB_SIZE);
        printf("%s}\n", tab);
      }
    }
  } else {
    printf("%snil\n", tab);
  }
}


int main()
{
  b_tree_t *tree;
  
  int seed = time(NULL);
  srand(seed);
  printf("seed=%d\n", seed);
  
  tree = b_new();
  
  int *claves = (int *) malloc(sizeof(int) * TOTAL_KEYS);
  for (int i = 0; i < TOTAL_KEYS; i++) {
    claves[i] = i;
  }
  shuffle(claves, TOTAL_KEYS);

  for (int i = 0; i < TOTAL_KEYS; i++) {
    b_add(tree, claves[i]);
  }
  
  int matches = 0;
  for (int i = 0; i < TOTAL_KEYS; i++) {
    if (b_find(tree, claves[i])) matches++; else break;
  }
  
  printf("%s [%d]\n",
         matches == TOTAL_KEYS ? "estan todos!" : "fallo :'(",
         matches);
  
  b_delete(tree);
  
  return 0;  
}
