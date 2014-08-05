
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "b-tree.h"


#define HEAVY_TEST 17


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


bool checkear_nodos (b_node_t *n) {
  int i;
  bool r, b;
  
  r = true;
  for (i = 0; i < n->used_keys && r; i++) {
    if (n->childs[i] != NULL) {
      assert(n->childs[i]->parent == n);
      r = r && (n->childs[i]->parent == n);
      if (n->childs[i]->used_keys > 0) {
        r = r && (n->childs[i]->keys[n->childs[i]->used_keys - 1] < n->keys[i])
              && (n->childs[i + 1] == NULL || n->keys[i] < n->childs[i + 1]->keys[0]);
      }
      b = checkear_nodos(n->childs[i]);
      assert(b);
      r = r && b;
    }
  }
  if (n->childs[i] != NULL) {
    assert(n->childs[i]->parent == n);
    r = r && (n->childs[i]->parent == n);
    b = checkear_nodos(n->childs[i]);
    assert(b);
    r = r && b;
  }
  
  return r;
}


bool test_invariantes (b_tree_t *t) {
  bool r;
  
  r = true;
  
  if (t == NULL || t->root == NULL)
    return r;
  
  r = r && t->root->parent == NULL;
  r = r && checkear_nodos(t->root);
  
  return r;
}


int main () {
  
  b_tree_t *tree;
  int a[HEAVY_TEST];
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
  /*printf("%s\n", test_invariantes(tree) ? "ANDAAAAAAAAA" : "algo feo :s");*/
  
  b_delete(tree);
  
  return 0;
  
}
