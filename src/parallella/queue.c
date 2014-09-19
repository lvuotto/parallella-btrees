#include "queue.h"
#include <stdlib.h>


static queue_node * queue_node_new();


queue * queue_new()
{
  queue *q = (queue *) malloc(sizeof(queue));
  q->top = NULL;
  q->last = NULL;
  
  return q;
}


bool empty(const queue *q)
{
  return q->top == NULL;
}


queue * enqueue(queue *q, void *v)
{
  queue_node *n = queue_node_new();
  n->value = v;
  if (empty(q)) {
    q->top = n;
    q->last = n;
  } else {
    q->last->next = n;
    q->last = n;
  }

  return q;
}


void * dequeue(queue *q)
{
  queue_node *n = q->top;
  q->top = n->next;
  void *v = n->value;
  free(n);

  return v;
}


void queue_delete(queue *q)
{
  queue_node *n = q->top;
  while (n != NULL) {
    queue_node *t = n->next;
    free(n);
    n = t;
  }
  free(q);
}


static queue_node * queue_node_new()
{
  queue_node *n = (queue_node *) malloc(sizeof(queue_node));
  n->value = NULL;
  n->next = NULL;

  return n;
}
