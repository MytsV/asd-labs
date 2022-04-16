#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/matrix_tools.h"

int is_directed; //чи є граф напрямленим
int node_count_graph; //кількість вершин
int condensed_size; //кількість вершин конденсованого графа

/*
    ~ДОПОМІЖНІ ФУНКЦІЇ~
*/

double **new_matrix(int n) {
    double **matrix = (double **)malloc(sizeof(double *) * n);

    for (int i = 0; i < n; i++)
    {
        double *row = (double *)malloc(sizeof(double) * n);
        matrix[i] = row;
        for (int j = 0; j < n; j++)
        {
            row[j] = 0;
        }
    }

    return matrix;
}

int *new_array(int n) {
    int *array = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        array[i] = 0;
    }
    return array;
}

double **matrix_square_multiply(int n, double **a, double **b) 
{
    double **multiplied = new_matrix(n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++) {
                multiplied[i][j] += a[k][j] * b[i][k];
            }
        }
    }

    return multiplied;
}

double **matrix_transpose(int n, double **matrix)
{
    double **transpose = new_matrix(n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            transpose[i][j] = matrix[j][i];
        }
    }

    return transpose;
}

//функції set повинні бути викликані з основного коду
void set_directed(int directed) {
    is_directed = directed;
}

void set_count(int n) {
    node_count_graph = n;
}

void output_array(int n, int array[n]) 
{
    if (n == 0) {
        printf("none\n");
        return;
    }
    
    printf("[");
    for (int i = 0; i < n; i++) 
    {
        if (i < n - 1) 
        {
            printf("%d, ", array[i]);
        } 
        else 
        {
            printf("%d]\n", array[i]);
        }
    }
}

int get_component_num(int n, int *components) //отримання загальної кількості компонентів
{
    int component_num = 0; //шукаємо максимум
    for (int i = 0; i < n; i++) 
    {
        if (components[i] > component_num) 
        {
            component_num = components[i];
        }
    }
    return component_num;
}

void output_components(int n, int *components) { //вивід компонентів зв'язку графу
    int component_num = get_component_num(n, components);

    for (int i = 1; i <= component_num; i++) {
        printf("Component %d: [ ", i);
        for (int j = 0; j < n; j++) {
            if (components[j] == i) {
                printf("%d ", j + 1);
            }
        }
        printf("]\n");
    }
}

/*
    ~ФУНКЦІЇ ВИЗНАЧЕННЯ СТЕПЕНІВ ВЕРШИН~
*/

int *get_undirected_degrees(int n, double **matrix) //визначає повні степені або напівстепені виходу вершин
{
    int *degrees = new_array(n);

    //ітеруємо рядки
    for (int i = 0; i < n; i++) 
    {
        degrees[i] = 0;
        for (int j = 0; j < n; j++) 
        {
            if (matrix[i][j]) 
            {
                degrees[i]++;
            }
        }
    }

    return degrees;
}

int *get_in_degrees(int n, double **matrix) { //визначає напівстепені входу вершин
    int *degrees = new_array(n);

    //ітеруємо стовпці
    for (int j = 0; j < n; j++) 
    {
        degrees[j] = 0;
        for (int i = 0; i < n; i++) 
        {
            if (matrix[i][j]) 
            {
                degrees[j]++;
            }
        }
    }

    return degrees;
}

/*
    ~ФУНКЦІЇ ВИЗНАЧЕННЯ ПАРАМЕТРІВ ГРАФУ, ЯКІ ЗАЛЕЖАТЬ ВІД СТЕПЕНІВ ВЕРШИН~
*/

void output_node_types(int n, int *degrees) 
{
    int leaf_count = 0;
    int isolated_count = 0;

    int leaves[n]; //висячі вершини
    int isolated[n]; //ізольовані вершини

    for (int i = 0; i < n; i++) 
    {
        if (degrees[i] == 0) 
        {
            isolated[isolated_count] = i + 1;
            isolated_count++;
        } 
        else if (degrees[i] == 1) 
        {
            leaves[leaf_count] = i + 1;
            leaf_count++;
        }
    }

    printf("Isolated: ");
    output_array(isolated_count, isolated);

    printf("Leaves: ");
    output_array(leaf_count, leaves);
}

void output_is_regular(int n, int *degrees) //визначає, чи граф є однорідним
{
    int is_regular = 1;
    for (int i = 1; i < n; i++) 
    {
        if (degrees[i] != degrees[i - 1]) //перевіряємо, чи всі вершини мають однакові степені
        {
            is_regular = 0;
            break;
        }
    }

    if (is_regular) 
    {
        printf("\nThe graph is regular. Degree = %d\n\n", degrees[0]);
    } 
    else 
    {
        printf("\nThe graph is not regular\n\n");
    }
}

/*
    ~ФУНКЦІЇ ВИВОДУ ВСІХ ПАРАМЕТРІВ, ПОВ'ЯЗАНИХ ЗІ СТЕПЕНЯМИ ВЕРШИН~
*/

int *get_total(int n, int *out_degrees, int *in_degrees) {
    int *total = new_array(n * 2);

    memcpy(total, out_degrees, n * sizeof(int));
    memcpy(total + n, in_degrees, n * sizeof(int));

    return total;
}

int *get_summed(int n, int *out_degrees, int *in_degrees) {
    int *summed = new_array(n);
    for (int i = 0; i < n; i++) {
        summed[i] = out_degrees[i] + in_degrees[i];
    }
    return summed;
}

void output_directed_degree_info(int n, double **matrix)
{
    printf("Out degrees: "); //напівстепені виходу
    int *out_degrees = get_undirected_degrees(n, matrix);
    output_array(n, out_degrees);

    printf("In degrees: "); //напівстепені входу
    int *in_degrees = get_in_degrees(n, matrix);
    output_array(n, in_degrees);

    int* total = get_total(n, out_degrees, in_degrees); //додаємо в один масив напівстепені
    output_is_regular(n * 2, total);
    free(total);

    int* summed = get_summed(n, out_degrees, in_degrees); //додаємо напівстепені докупи
    output_node_types(n, summed);
    free(summed);
    
    free(out_degrees);
    free(in_degrees);
}

void output_degree_operations(double **matrix) {
    printf("\nOutputting degrees for every node:\n");

    if (!is_directed) {
        int *degrees = get_undirected_degrees(node_count_graph, matrix);
        output_array(node_count_graph, degrees);
        output_is_regular(node_count_graph, degrees);
        output_node_types(node_count_graph, degrees);

        free(degrees);
    } else {
        output_directed_degree_info(node_count_graph, matrix);
    }
}

/*
    ~ФУНКЦІЇ ВИВОДУ ШЛЯХІВ~
*/

void get_paths(int length, int n, double **matrix) 
{
    if (length < 1 || length > 3) {
        printf("unsupported length!\n");
        return;
    }

    printf("{\n");
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            if (!matrix[i][j]) continue;

            if (length == 1)
            {
                printf("%d->%d\n", i + 1, j + 1);
                continue;
            }

            for (int k = 0; k < n; k++) 
            {
                if (!matrix[j][k]) continue;

                if (length == 2) 
                {
                    printf("%d->%d->%d\n", i + 1, j + 1, k + 1);
                    continue;
                } 
                for (int l = 0; l < n; l++) 
                {
                    if (!matrix[k][l]) continue;

                    printf("%d->%d->%d->%d\n", i + 1, j + 1, k + 1, l + 1);
                }
            }
        }
    }
    printf("}\n");
}

void output_path_operations(double **matrix) {
    printf("\nMatrix of power 2: \n");
    double **matrix_2 = matrix_square_multiply(node_count_graph, matrix, matrix);
    output_matrix(node_count_graph, node_count_graph, matrix_2);

    printf("\nPaths of length 2: \n");
    get_paths(1, node_count_graph, matrix_2);

    printf("\nMatrix of power 3: \n");
    double **matrix_3 = matrix_square_multiply(node_count_graph, matrix, matrix_2);
    output_matrix(node_count_graph, node_count_graph, matrix_3);

    printf("\nPaths of length 3: \n");
    get_paths(1, node_count_graph, matrix_3);
}

/*
    ~ФУНКЦІЇ ДОСЯЖНОСТІ~
*/

//пошук в глибину для знаходження компонентів зв'язку
void search_components(double **matrix, int index, int component_num, int *used, int *components) {
    used[index] = 1; //позначаємо, що ця вершина була проглянута
    components[index] = component_num; //позначаємо, що ця вершина відноситься до певного компонента

    for (int i = 0; i < node_count_graph; i++) {
        if (!used[i] && matrix[index][i]) {
            search_components(matrix, i, component_num, used, components);
        }
    }
}

int *get_connection_components(int n, double **matrix) //отримання компонентів зв'язку, до яких належить кожна з вершин
{
    int component_num = 1;

    int *used = new_array(n);
    for (int i = 0; i < n; i++) {
        used[i] = 0;
    }

    int *components = new_array(n);

    for (int i = 0; i < n; i++) 
    {
        if (!used[i]) 
        {
            search_components(matrix, i, component_num, used, components);
            component_num++;
        }
    }

    free(used);
    return components;
}

double **get_connection_matrix(int n, double **reachability) //отримання матриці зв'язності
{
    double **transpose = matrix_transpose(n, reachability);
    
    double **connection_matrix = new_matrix(n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            connection_matrix[i][j] = reachability[i][j] && transpose[i][j];
        }
    }

    free(transpose);

    return connection_matrix;
}

double **get_reachability_matrix(int n, double **matrix) //отримання матриці досяжності
{
    double **reachability = new_matrix(n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            reachability[i][j] = matrix[i][j];
        }
    }

    double **powered = matrix_square_multiply(node_count_graph, matrix, matrix);

    for (int time = 0; time < n - 1; time++) //операція транзитивного замикання
    {
        for (int i = 0; i < n; i++)
        {   
            for (int j = 0; j < n; j++)
            {
                reachability[i][j] = reachability[i][j] || powered[i][j];
            }
        }
        double **temp = matrix_square_multiply(node_count_graph, powered, matrix);
        free(powered);
        powered = temp;
    }

    return reachability;
}

int get_condensed_matrix_size() { //отримання кількості вершин графа конденсації
    return condensed_size;
}

double **get_condensed_matrix(int n, double **matrix) { //отримання матриці суміжності графа конденсації
    double **reachability = get_reachability_matrix(n, matrix);
    double **connection_matrix = get_connection_matrix(n, reachability);
    int *components = get_connection_components(n, connection_matrix);

    output_components(n, components);
    free_matrix(n, connection_matrix);
    free_matrix(n, reachability);

    int component_num = get_component_num(n, components);
    condensed_size = component_num;

    double **condensed = new_matrix(component_num); //генеруємо матрицю

    for (int i = 0; i < component_num; i++) 
    {
        for (int j = 0; j < n; j++)
        {
            if (components[j] == i + 1) //для кожного компоненту переглядаємо вершини, що входять в нього
            {
                for (int k = 0; k < n; k++) {
                    if (components[k] != i + 1) { //перевіряємо, з вершинами яких з інших компонентів суміжні вершини розглянутого компоненту
                        if (matrix[j][k]) { //перевіряємо напрям з'єднаня
                            condensed[i][components[k] - 1] = 1;
                        } else if (matrix[k][j]) {
                            condensed[components[k] - 1][i] = 1;
                        }
                    }
                }
            }
        }
    }

    printf("\n");
    output_matrix(component_num, component_num, condensed);
    free(components);

    return condensed;
}

/*
    ~ВИВІД ПАРАМЕТРІВ ДОСЯЖНОСТІ~
*/

void output_reachability_operations(double **matrix) {
    double **reachability = get_reachability_matrix(node_count_graph, matrix);
    printf("\nReachability matrix:\n");
    output_matrix(node_count_graph, node_count_graph, reachability);

    double **connection_matrix = get_connection_matrix(node_count_graph, reachability);
    printf("\nConnection matrix:\n");
    output_matrix(node_count_graph, node_count_graph, connection_matrix);

    int *components = get_connection_components(node_count_graph, connection_matrix);
    printf("\nComponents themselves:\n");
    output_components(node_count_graph, components);
    
    free_matrix(node_count_graph,reachability);
    free_matrix(node_count_graph, connection_matrix);

    free(components);
}

/*
    ~ЗАГАЛЬНИЙ ВИВІД~
*/

void additional_output(int type, double **matrix) 
{
    output_degree_operations(matrix);
    if (type == 2) { //перевіряємо, чи матриця модифікована
        output_path_operations(matrix);
        output_reachability_operations(matrix);
    }
}