#include "queue.h"
#include <stdlib.h>
#include <assert.h>


static queue_node * queue_node_new();


queue * queue_new()
{
  queue *q = (queue *) malloc(sizeof(queue));
  q->front_ = NULL;
  q->back_ = NULL;
  
  return q;
}


queue * enqueue(queue *q, void *v)
{
  queue_node *n = queue_node_new();
  n->value = v;
  if (queue_empty(q))
    q->front_ = n;
  else
    q->back_->next = n;
  q->back_ = n;

  return q;
}


void * dequeue(queue *q)
{
  assert(!queue_empty(q));

  queue_node *n = q->front_;
  q->front_ = n->next;
  void *v = n->value;
  free(n);

  return v;
}


void queue_delete(queue *q)
{
  queue_node *n = q->front_;
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
