#include <stdio.h>

double x;

double get_multiplier(int n)
{
    return - x * x * (2 * n - 1) * (2 * n - 1) / (4 * n * n + 2 * n);
}

double get_sum(double argument, int count)
{
    x = argument;

    double sum = argument;
    double element = argument;
    
    for (int i = 2; i <= count; i++) {
        element *= get_multiplier(i - 1);
        sum += element;
    }
    return sum;
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

    double sum = get_sum(argument, count);

    printf("The result is %.30lf", sum);

    return 0;
}