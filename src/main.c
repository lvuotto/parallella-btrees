
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "b-tree.h"


#define HEAVY_TEST (1 << 20)
#define TAB_SIZE 4


void node_replace (b_node_t *node, b_key_t key, int index) {
  int j;
  
  assert(0 <= index && index < node->used_keys);
  
  if (key < node->keys[index]) {
    for (j = index; j > 0 && key < node->keys[j - 1]; j--) {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
    }
  } else {
    for (j = index; j < node->used_keys - 1 && key > node->keys[j + 1]; j++) {
      node->keys[j] = node->keys[j + 1];
      node->children[j] = node->children[j + 1];
    }
  }
  
  node->keys[j] = key;
}


void print_node (b_node_t *node, int t) {
  int i, j;
  char tab[256];
  
  for (j = 0; j < t; j++) {
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
    for (i = 0; i < node->used_keys; i++) {
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


int main () {
  
  b_tree_t *tree;
  int a[HEAVY_TEST];
  int i, j, m, c;
  
  c = time(NULL);
  srand(time(NULL));
  
  printf("seed=%d\n", c);
  
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
  
  c = 0;
  for (i = 0; i < HEAVY_TEST; i++) {
    m = b_find(tree, a[i]);
    if (!m) break;
    else c++;
  }
  
  printf("%s [%d]\n", m ? "estan todos!" : "fallo :'(", c);
  
  b_delete(tree);
  
  return 0;
  
}
