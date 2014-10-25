#include <stdio.h>
#include <time.h>
#include "b-tree.h"


#define TEST_SIZE 0x00010000


int main()
{
  /* COMIENZO busqueda en paralelo */
  b_tree_t *tree;
  tree = b_new();
  int b[E_CORES];
  for (int i = 0; i < TEST_SIZE; i++) {
    b_add(tree, i+1);
  }
  share(tree, &mem);

  clock_t dt;
  double tiempo;
  log("Realizando la busqueda...");
  dt = clock();
  for (int total = 0; total < TEST_SIZE; total += E_CORES) {
    for (int j = 0; j < E_CORES; j++)
      b[j] = 1+j + total;
    b_status_t *response = b_find_parallel(&platform, &device, &mem, btmi, b);
    free(response);
  }
  dt = clock() - dt;
  log("Busqueda completada.");
  tiempo = ((double) dt) / CLOCKS_PER_SEC;
  printf("Tiempo total: %.5fs\n", tiempo);
  b_delete(tree);
  /* FIN busqueda en paralelo */

  return 0;  
}
