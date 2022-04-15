#include <stdio.h>

double x;

double get_multiplier(int n) 
{
    return - x * x * (2 * n - 1) * (2 * n - 1) / (4 * n * n + 2 * n);
}

double recursion_sum(int n, double *sum) 
{
    if (n == 1) 
    {
        *sum += x;
        return x;
    }
    double element = recursion_sum(n - 1, sum) * get_multiplier(n - 1);
    printf("F%d %lf\n", n, element);
    *sum += element;
    return element;
}

void get_sum(double argument, int count, double *sum) 
{
    *sum = 0;
    x = argument;
    recursion_sum(count, sum);
}

int main() 
{
    double argument = -1;
    while (argument >= 1 || argument <= -1)
    {
        printf("Input real number x, |x| < 1\n");
        scanf("%lf", &argument);
    }

    unsigned int count;
    printf("Input integer n\n");
    scanf("%d", &count);

    double sum;
    get_sum(argument, count, &sum);

    printf("The result is %.30lf", sum);

    return 0;
}