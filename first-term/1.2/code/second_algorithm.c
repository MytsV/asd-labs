#include <stdio.h>
#include <math.h>

int operation_count = 0; //counting only arithmetic operations, excluding operation_count increment

int main() {
    int n;
    printf("Input N value: ");
    scanf("%d", &n);    

    double sum = 0; // =
    int last_product = 1; // =
    operation_count += 2;

    for (int i = 1; i <= n; i++) { // <=, ++
        //jump and i assignment (once)
        last_product *= 2 * i + 1; // *=, *, +

        double divisor = 3 * i * log(i + 1); // =, *, *, log, +

        sum += last_product / divisor; // +=, /

        operation_count += 13;
    }
    printf("Result: %lf", sum);
    printf("\n");
    printf("Count of perfomed operations: %d", operation_count);

    return 0;
}