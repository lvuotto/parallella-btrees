#include <stdio.h>
#include <stdlib.h>
#include "b-tree.h"


#define MAX_TESTS (4 << 20)


void shuffle(int *a, size_t n);
size_t cant_nodos(const b_tree_t *t);
size_t contar(const b_node_t *n);


int main()
{
  srand(176);

  printf("sizeof(b_tree_t) = %ub\n", sizeof(b_tree_t));
  printf("sizeof(b_node_t) = %ub [%u claves]\n", sizeof(b_node_t), B_MAX_KEYS);
  puts("");
  
  b_tree_t *tree = b_new();
  for (size_t i = 1; i <= MAX_TESTS; i <<= 1) {
    int *a = (int *) malloc(i*sizeof(int));
    for (unsigned int j = 0; j < i; j++) {
      a[j] = j;
    }
    shuffle(a, i);

    for (unsigned int j = 0; j < i; j++) {
      b_add(tree, j);
    }
    
    size_t cant = cant_nodos(tree);
    printf("size: %ub [~ %.2fMB], %u nodos, %u claves, proporcion %.3f\n",
           sizeof(b_node_t)*cant + sizeof(b_tree_t),
           (sizeof(b_node_t)*cant + sizeof(b_tree_t)) / 1048576.0,
           cant,
           i,
           cant / (double) i);
    free(a);
  }
  b_delete(tree);

  return 0;
}


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


size_t cant_nodos(const b_tree_t *t)
{
  return contar(t->root);
}

size_t contar(const b_node_t *n)
{
  if (n == NULL)
    return 0;
  
  size_t r = 0;
  for (int i = 0; i <= n->used_keys; i++) {
    r += contar(n->children[i]);
  }
  return 1 + r;
}
