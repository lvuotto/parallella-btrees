#ifndef __QUEUE_H__
#define __QUEUE_H__


#include <stdbool.h>


/**
 * TODO:
 *  - Chequear errores.
 **/

typedef struct queue_      queue;
typedef struct queue_node_ queue_node;

struct queue_ {
  queue_node *top;
  queue_node *last;
};

struct queue_node_ {
  void       *value;
  queue_node *next;
};


queue * queue_new();
bool empty(const queue *q);
queue * enqueue(queue *q, void *v);
void * dequeue(queue *q);
void queue_delete(queue *q);


inline void * top(const queue *q)
{
  return q->top->value;
}


#endif /* __QUEUE_H__ */
