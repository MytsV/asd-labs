#ifndef MATRIX_TOOLS_H_
# define MATRIX_TOOLS_H_

extern const int NODE_COUNT;

double **get_matrix(int type);
void output_matrix(int n, int m, double **matrix);
void to_undirected(double **matrix);
void free_matrix(int n, double **matrix);

#endif