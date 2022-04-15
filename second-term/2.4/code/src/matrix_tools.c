#include <stdlib.h>
#include <stdio.h>

const int RAND_LIMIT = 2;
/*
parameters for randomization
*/ 
const int N1 = 1;
const int N2 = 2;
const int N3 = 1;
const int N4 = 9;

const int NODE_COUNT = 10 + N3;

double get_seed()
{
    return N1 * 1000 + N2 * 100 + N3 * 10 + N4;
}

double get_coef(int type)
{
    if (type == 2) {
        return 1 - N3 * 0.005 - N4 * 0.005 - 0.27;
    } else {
        return 1 - N3 * 0.01 - N4 * 0.01 - 0.3;
    }
}

double ranged_rand()
{
    return (double)rand() / ((double)RAND_MAX / RAND_LIMIT);
}

double **randm(int n, int m)
{
    double **matrix = (double **)malloc(sizeof(double *) * n);
    for (int i = 0; i < n; i++)
    {
        double *row = (double *)malloc(sizeof(double) * m);
        matrix[i] = row;
        for (int j = 0; j < m; j++)
        {
            row[j] = ranged_rand();
        }
    }
    return matrix;
}

void mulmr(double coef, int n, int m, double **matrix)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrix[i][j] = matrix[i][j] * coef >= 1 ? 1 : 0; 
        }
    }
}

void output_matrix(int n, int m, double **matrix)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            printf("%.0f ", matrix[i][j]);
        }
        printf("\n");
    }
}

double **test_matrix()
{
    double matrix_real[11][11] = 
    {
        0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    double **matrix = (double **)malloc(sizeof(double *) * NODE_COUNT);
    for (int i = 0; i < NODE_COUNT; i++) 
    {
        double *row = (double *)malloc(sizeof(double) * NODE_COUNT);
        matrix[i] = row;
        for (int j = 0; j < NODE_COUNT; j++) 
        {
            row[j] = matrix_real[i][j];
        }
    }

    return matrix;
}

void to_undirected(double **matrix) 
{
    for (int i = 0; i < NODE_COUNT; i++) 
    {
        for (int j = 0; j < NODE_COUNT; j++) 
        {
            if (matrix[i][j] == 1) 
            {
                matrix[j][i] = 1;
            }
        }
    }
}

double **get_matrix(int type) 
{
    return test_matrix();
    srand(get_seed());
    double **matrix = randm(NODE_COUNT, NODE_COUNT);
    mulmr(get_coef(type), NODE_COUNT, NODE_COUNT, matrix);
    return matrix;
}

void free_matrix(int n, double **matrix) {
    for (int i = 0; i < n; i++) 
    {
        free(matrix[i]);
    }
    free(matrix);
}