#include <stdlib.h>
#include <stdio.h>

typedef struct node {
    unsigned int value;
    struct node *next;
} node_t;

node_t* create_list(unsigned int value) {
    node_t *head = NULL;
    head = (node_t*) malloc(sizeof(node_t));
    if (head == NULL) {
        return NULL;
    }

    head->value = value;
    head->next = NULL;
    return head;
}

void push(node_t *head, unsigned int value) {
    node_t *ptr = head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }

    ptr->next = create_list(value);
}

node_t* get_middle(node_t *list) {
    int iterations = 0;
    node_t *slow = list;
    node_t *fast = list;

    while (fast != NULL && fast->next != NULL && fast->next->next != NULL) {
        iterations++;
        slow = slow->next;
        fast = fast->next->next;
    }

    printf("Iterations: %d\n", iterations);
    return slow;
}

node_t* get_middle_brute(node_t *list) {
    int iterations = 0;
    int length = 0;
    node_t *ptr = list;

    while (ptr != NULL) {
        length++;
        ptr = ptr->next;
        iterations++;
    }

    length /= 2;
    length--;
    ptr = list;
    while (length > 0) {
        length--;
        ptr = ptr->next;
        iterations++;
    }

    printf("Iterations: %d\n", iterations);
    return ptr;
}

node_t* get_list(int length) {
    node_t *list = create_list(1);
    for (int i = 2; i <= length; i++) {
        push(list, i);
    }
    return list;
}

void free_list(node_t *list) {
    node_t *temp = NULL;

    while (list != NULL) {
        temp = list;
        list = list->next;
        free(temp);
    }
}

int main() {
    node_t *list = get_list(500);
    printf("Pointers:\n");
    printf("Value: %d\n", get_middle(list)->value);
    printf("Brute:\n");
    printf("Value: %d\n", get_middle_brute(list)->value);

    free_list(list);
    return 0;
}