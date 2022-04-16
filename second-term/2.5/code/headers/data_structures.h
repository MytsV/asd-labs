#ifndef DATA_STRUCTURES_H_
# define DATA_STRUCTURES_H_

typedef struct stack
{
    int last_index;
    int *elements;
    int capacity;
}
my_stack_t;

typedef struct queue {
    int front, rear, size;
    int capacity;
    int* array;
}
my_queue_t;

my_stack_t *new_stack(int capacity);
void push_stack(my_stack_t *stack, int element);
int pop_stack(my_stack_t *stack);
int peek_stack(my_stack_t *stack);
my_queue_t *new_queue(int capacity);
void enqueue_queue(my_queue_t* queue, int item);
int dequeue_queue(my_queue_t* queue);
int peek_queue(my_queue_t* queue);

#endif