#include <stdio.h>
#include <math.h>

int operation_count = 0; //counting only arithmetic operations, excluding operation_count increment

int main() {
    int n;
    printf("Input N value: ");
    scanf("%d", &n);    

    double sum = 0; // =
    for (int i = 1; i <= n; i++) { // <=, ++
        //jump and i assignment (once)
        double product = 1; // =

        for (int j = 1; j <= i; j++) { // <=, ++
            //jump and j assignment (once)          
            product *= 2 * j + 1; // *=, *, +
            operation_count += 6;
        }

        double divisor = 3 * i * log(i + 1); // =, *, *, log, +
        sum += product / divisor; // +=, /
        
        operation_count += 11;
    }

    printf("Result: %lf", sum);
    printf("\n");
    printf("Count of perfomed operations: %d", operation_count);

    return 0;
}