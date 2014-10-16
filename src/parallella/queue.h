#ifndef __QUEUE_H__
#define __QUEUE_H__


#include <assert.h>
#include <stddef.h>


/**
 * TODO:
 *  - Chequear errores.
 **/

typedef struct queue_      queue;
typedef struct queue_node_ queue_node;

struct queue_ {
  queue_node *front_;
  queue_node *back_;
};

struct queue_node_ {
  void       *value;
  queue_node *next;
};


queue * queue_new();
queue * enqueue(queue *q, void *v);
void * dequeue(queue *q);
void queue_delete(queue *q);


/* bool queue_empty(const queue *q); */
#define queue_empty(q) (q->front_ == NULL)


inline void * front(const queue *q)
{
  assert(!queue_empty(q));
  return q->front_->value;
}


inline void * back(const queue *q)
{
  assert(!queue_empty(q));
  return q->back_->value;
}


#endif /* __QUEUE_H__ */
