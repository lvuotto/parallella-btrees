#include <stdio.h>
#include <time.h>
#include "b-tree.h"


#define TEST_SIZE 0x00010000


int main()
{
  /* COMIENZO busqueda secuencial */
  b_tree_t *tree;
  tree = b_new();
  for (int i = 0; i < TEST_SIZE; i++) {
    b_add(tree, i+1);
  }

  clock_t dt;
  double tiempo;
  printf("Realizando la busqueda...");
  dt = clock();
  for (int total = 1; total <= TEST_SIZE; total++) {
    b_find(tree, total);
  }
  dt = clock() - dt;
  printf("Busqueda completada.");
  tiempo = ((double) dt) / CLOCKS_PER_SEC;
  printf("Tiempo total: %.5fs\n", tiempo);
  b_delete(tree);
  /* FIN busqueda secuencial */

  return 0;  
}
