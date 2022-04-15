#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <time.h> 

double a;
double b;
int n;
const double MAX_NUMBER = 100;

double rand_finite_double() { 
    int multiplier = MAX_NUMBER * 2;
    return ((double) rand() / RAND_MAX) * multiplier - 100;
}

double getZ(double y) {
    if (y > -50 && y < 0) {
        return 10 * a - y;
    } else {
        return 1 + y * y;
    }
}

double get_final_result(double zResults[]) {
    double max = -DBL_MAX;
    for (int i = 0; i < n; i++) {
        int multiplier = (i + 1) % 2  == 0 ? 1 : -1;
        double value = multiplier * b + zResults[i];
        if (value > max) {
            max = value;
        }
    }
    return max;
}

void get_double_input(char input_text[], double *out) {
    printf("%s", input_text);
    scanf("%lf", out);
}

void get_int_input(char input_text[], int *out) {
    printf("%s", input_text);
    scanf("%d", out);
}

void init_random_numbers(double randomNumbers[]) {
    for (int i = 0; i < n; i++) {
        randomNumbers[i] = rand_finite_double();
        printf("The %d random number is %lf\n", i + 1, randomNumbers[i]);
    }
}

void init_z_results(double randomNumbers[], double zResults[]) {
    for (int i = 0; i < n; i++) {
        zResults[i] = getZ(randomNumbers[i]);
        printf("The %d z result is %lf\n", i + 1, zResults[i]);
    }
}

int main() {
    srand(time(0));  //setting a seed so that the numbers don't repeat each time the program runs

    get_double_input("Input value a: ", &a);
    get_double_input("Input value b: ", &b);
    get_int_input("Input value n: ", &n);

    double randomNumbers[n];
    
    init_random_numbers(randomNumbers);

    double zResults[n];

    init_z_results(randomNumbers, zResults);

    double finalResult = get_final_result(zResults);
    printf("The final result is %lf", finalResult);

    return 0;
}