#include <stdio.h>

void print_matrix(int m, int n, int matrix[m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void scan_matrix(int m, int n, int matrix[m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }
}

void sort(int m, int matrix[m][m]) {
    for (int i = 1; i < m; i++) {
        int element = matrix[i][i];
        int j = i;
        while (matrix[j - 1][j - 1] > element && j > 0) {
            matrix[j][j] = matrix[j - 1][j - 1];
            j--;
        }
        matrix[j][j] = element;
    }
}

int main() {
    int n;
    scanf("%d", &n);
    
    int matrix[n][n];
    scan_matrix(n, n, matrix);
    print_matrix(n, n, matrix);

    sort(n, matrix);
    
    printf("Sorted matrix: \n");
    print_matrix(n, n, matrix);

    return 0;
}