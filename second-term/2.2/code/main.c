#include <stdlib.h>
#include <stdio.h>

typedef struct node
{
    unsigned int value;
    struct node *next;
} node_t;

node_t *create_list(unsigned int value)
{
    node_t *head = NULL;
    head = (node_t *) malloc(sizeof(node_t));
    if (head == NULL)
    {
        return NULL;
    }

    head->value = value;
    head->next = NULL;
    return head;
}

void push(node_t *head, unsigned int value)
{
    node_t *ptr = head;

    while (ptr->next != NULL)
    {
        ptr = ptr->next;
    }

    ptr->next = create_list(value);
}

void free_list(node_t *head)
{
    node_t *temp = NULL;

    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void iterate_list(node_t *head, void (*func)(node_t *))
{
    while (head != NULL)
    {
        func(head);
        head = head->next;
    }
}

void print_node(node_t *node)
{
    if (node->next != NULL)
    {
        printf("%d, ", node->value);
    }
    else
    {
        printf("%d", node->value);
    }
}

void print_list(node_t *head)
{
    printf("[");
    iterate_list(head, print_node);
    printf("]\n");
}

node_t *get_middle(node_t *head)
{
    node_t *slow = head;
    node_t *fast = head;

    while (fast != NULL && fast->next != NULL && fast->next->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;
    }

    return slow;
}

void insert_at(node_t *node, node_t *inserted_list)
{
    if (node->next == NULL)
    {
        return;
    }

    node_t *temp = node->next;
    while (inserted_list != NULL && inserted_list->next != NULL)
    {
        node->next = inserted_list;
        inserted_list = inserted_list->next;
        node = node->next;
    }

    node->next = inserted_list;
    inserted_list->next = temp;
}

node_t *read_list(int size)
{
    node_t *list = NULL;

    while (size > 0)
    {
        unsigned int value;
        scanf("%d", &value);
        
        if (list == NULL)
        {
            list = create_list(value);
        } 
        else
        {
            push(list, value);
        }
        
        size--;
    }

    return list;
}

int main()
{
    int n;
    printf("Input n: \n");
    scanf("%d", &n);

    printf("Input %d elements of main list\n", 2 * n);
    node_t *main_list = read_list(2 * n);

    printf("Input %d elements of inserted list\n", n);
    node_t *inserted_list = read_list(n);

    printf("Original main list\n");
    print_list(main_list);

    printf("List to insert\n");
    print_list(inserted_list);

    insert_at(get_middle(main_list), inserted_list);

    printf("List after insertion\n");
    print_list(main_list);

    free_list(main_list);
    return 0;
}