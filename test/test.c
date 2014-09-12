#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <b-tree.h>


#define HEAVY_TEST (1 << 20)
#define eprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

#define MSG_FIND_OK "Todas las claves estan en el arbol."
#define MSG_FIND_FAIL "Hay claves que no estan en el arbol."
#define MSG_OK   "El arbol esta bien armado."
#define MSG_FAIL "El arbol tiene fallas de estructura."


b_key_t min_key(b_node_t *n);
b_key_t max_key(b_node_t *n);
bool esta_ordenado(b_node_t *n);
bool respeta_rangos(b_node_t *n);
bool checkear_nodo(b_node_t *n);
bool test_invariantes(b_tree_t *t);
bool hijos_cumplen(b_node_t *n);

int a[HEAVY_TEST];

int main()
{
  b_tree_t *tree;
  int i, j, m, c;
  
  m = time(NULL);
  srand(m);
  printf("seed=%d\n", m);
  
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
    if (b_find(tree, a[i])) c++;
  }
  
  printf("%s\n", c == HEAVY_TEST ? MSG_FIND_OK : MSG_FIND_FAIL);
  printf("%s\n", test_invariantes(tree) ? MSG_OK : MSG_FAIL);
  
  b_delete(tree);
  
  return 0;
}


b_key_t min_key(b_node_t *n)
{
  b_node_t *b = n;
  while (b->children[0] != NULL) {
    b = b->children[0];
  }
  
  return b->keys[0];
}


b_key_t max_key(b_node_t *n)
{
  b_node_t *b = n;
  while (b->children[b->used_keys - 1] != NULL) {
    b = b->children[b->used_keys - 1];
  }
  
  return b->keys[b->used_keys - 1];
}


bool esta_ordenado(b_node_t *n)
{
  bool r = true;
  if (n == NULL) return r;
  
  for (int i = 0; i < n->used_keys - 1 && r; i++)
    r = r && (n->keys[i] < n->keys[i + 1]);
  
  if (!r)
    eprintf("Nodo %p no esta ordenado\n", (void *) n);
  
  return r;
}


bool respeta_rangos(b_node_t *n)
{
  bool r = true;
  if (n == NULL) return r;
  
  for (int i = 0; i < n->used_keys && r; i++) {
    r = r &&
        (n->children[i] == NULL ||
         max_key(n->children[i]) < n->keys[i]) &&
        (n->children[i + 1] == NULL ||
         n->keys[i] < min_key(n->children[i + 1]));
  }
  
  if (!r)
    eprintf("Nodo %p no respeta rangos\n", (void *) n);
  
  return r;
}


bool hijos_cumplen(b_node_t *n)
{
  bool r = true;
  if (n == NULL) return r;
  
  for (int i = 0; i <= n->used_keys && r; i++)
    if (n->children[i] != NULL)
      r = r && n->children[i]->parent == n && checkear_nodo(n->children[i]);
  
  if (!r)
    eprintf("Los hijos de %p no cumplen\n", (void *) n);
  
  return r;
}


bool checkear_nodo(b_node_t *n)
{
  return esta_ordenado(n) && respeta_rangos(n) && hijos_cumplen(n);
}


bool test_invariantes(b_tree_t *t)
{
  return t == NULL ||
         t->root == NULL ||
         (t->root->parent == NULL && checkear_nodo(t->root));
}
