#include <gtk/gtk.h>
#include <cairo.h>

#ifndef GRAPH_OPERATIONS_H_
# define GRAPH_OPERATIONS_H_

void set_directed(int directed);
void set_count(int n);
void additional_output(int type, double **matrix);
double **get_condensed_matrix(int n, double **matrix);
int get_condensed_matrix_size();

#endif