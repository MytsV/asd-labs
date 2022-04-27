#ifndef MATRIX_TOOLS_H_
# define MATRIX_TOOLS_H_

extern const int NODE_COUNT;

double **get_matrix();
void output_matrix(int n, int m, double **matrix);
void to_undirected(double **matrix);
void free_matrix(int n, double **matrix);
double **get_weight_matrix(double **a_matrix, int n, int m);

#endif