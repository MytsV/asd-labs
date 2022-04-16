#include <stdlib.h>

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

my_stack_t *new_stack(int capacity) 
{
    my_stack_t *stack = (my_stack_t *)malloc(sizeof(my_stack_t));
    stack->last_index = -1;
    stack->elements = (int *)malloc(sizeof(int) * capacity);
    stack->capacity = capacity;
    return stack;
}

void push_stack(my_stack_t *stack, int element) 
{
    if (stack->last_index >= (stack->capacity - 1))
    {
        return;
    }

    stack->last_index++;
    stack->elements[stack->last_index] = element;
}

int pop_stack(my_stack_t *stack)
{
    if (stack->last_index == -1) 
    {
        return -1;
    }
    
    int element = stack->elements[stack->last_index];
    stack->last_index--;

    return element;
}

int peek_stack(my_stack_t *stack)
{
    if (stack->last_index == -1) 
    {
        return -1;
    }
    return stack->elements[stack->last_index];
}

my_queue_t *new_queue(int capacity)
{
    my_queue_t *queue = (my_queue_t *)malloc(sizeof(my_queue_t));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    return queue;
}
 
void enqueue_queue(my_queue_t* queue, int item)
{
    if (queue->size == queue->capacity)
    {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
 
int dequeue_queue(my_queue_t* queue)
{
    if (queue->size == 0)
    {
        return -1;
    }
    int element = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return element;
}

int peek_queue(my_queue_t* queue)
{
    if (queue->size == 0)
    {
        return -1;
    }
    return queue->array[queue->front];
}