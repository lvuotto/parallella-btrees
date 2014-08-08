
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "b-tree.h"


#define HEAVY_TEST (1 << 19)
#define TAB_SIZE 4


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


bool checkear_nodos (b_node_t *n) {
  int i;
  bool r, b;
  
  r = true;
  for (i = 0; i < n->used_keys && r; i++) {
    if (n->children[i] != NULL) {
      assert(n->children[i]->parent == n);
      r = r && (n->children[i]->parent == n);
      if (n->children[i]->used_keys > 0) {
        r = r && (n->children[i]->keys[n->children[i]->used_keys - 1] < n->keys[i])
              && (n->children[i + 1] == NULL || n->keys[i] < n->children[i + 1]->keys[0]);
      }
      b = checkear_nodos(n->children[i]);
      assert(b);
      r = r && b;
    }
  }
  if (n->children[i] != NULL) {
    assert(n->children[i]->parent == n);
    r = r && (n->children[i]->parent == n);
    b = checkear_nodos(n->children[i]);
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
  int i, j, m, c;
  
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
    /*printf("Voy a agregar %d...\n", a[i]);*/
    b_add(tree, a[i]);
    /*getchar();
    print_node(tree->root, 0);*/
  }
  
  /*print_node(tree->root, 0);*/
  
  c = 0;
  for (i = 0; i < HEAVY_TEST; i++) {
    m = b_find(tree, a[i]);
    /*printf("[%d] -> %s\n", a[i], m ? "ok" : "no esta :(");*/
    if (!m) break;
    else c++;
  }
  
  printf("%s [%d]\n", m ? "estan todos!" : "fallo :'(", c);
  /*printf("%s\n", test_invariantes(tree) ? "ANDAAAAAAAAA" : "algo feo :s");*/
  
  b_delete(tree);
  
  return 0;
  
}
