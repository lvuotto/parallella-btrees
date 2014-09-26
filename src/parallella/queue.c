#include "queue.h"
#include <stdlib.h>


static queue_node * queue_node_new();


queue * queue_new()
{
  queue *q = (queue *) malloc(sizeof(queue));
  q->front_ = NULL;
  q->back_ = NULL;
  
  return q;
}


bool empty(const queue *q)
{
  return q->front_ == NULL;
}


queue * enqueue(queue *q, void *v)
{
  queue_node *n = queue_node_new();
  n->value = v;
  if (empty(q)) {
    q->front_ = n;
    q->back_ = n;
  } else {
    q->back_->next = n;
    q->back_ = n;
  }

  return q;
}


void * dequeue(queue *q)
{
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
