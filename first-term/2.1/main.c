#include <stdio.h>

const int RANGE_UPPER = 5;
const int RANGE_LOWER = 0;

struct result {
    int row;
    int column;
} typedef result_t;

int is_in_range(double element) {
    return element >= RANGE_LOWER && element <= RANGE_UPPER;
}

void print_matrix(int m, int n, double matrix[m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%15lf ", matrix[i][j]);
        }
        printf("\n");
    }
}

void scan_matrix(int m, int n, double matrix[m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%lf", &matrix[i][j]);
        }
    }
}

result_t find_value(int m, int n, double matrix[m][n]) {
    result_t result;
    result.row = -1;
    result.column = -1;

    for (int column_idx = 0; column_idx < n; column_idx++) {
        int begin = 0;
        int end = n - 1;

        while (begin <= end) {
            int mid = (begin + end) / 2;
            double element = matrix[mid][column_idx];

            if (is_in_range(element)) {
                result.row = mid;
                result.column = column_idx;
                break;
            } else if (element > RANGE_UPPER) {
                begin = mid + 1;
            } else {
                end = mid - 1;
            }
        }

        if (result.row >= 0 && result.column >= 0) {
            break;
        }
    }

    return result;
}

int main() {
    int m;
    scanf("%d", &m);

    int n;
    scanf("%d", &n);
    
    double matrix[m][n];
    scan_matrix(m, n, matrix);
    print_matrix(m, n, matrix);

    result_t result = find_value(m, n, matrix);
    
    if (result.row < 0 || result.column < 0) {
        printf("Haven't found any value in range [0,5]");
    } else {
        printf("Found a value in range [0,5]: %lf in row number %d and column number %d\n", matrix[result.row][result.column], result.row + 1, result.column + 1);
    }

    return 0;
}