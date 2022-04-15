#include <stdio.h>

double x;
int n;

double get_multiplier(int n) 
{
    return - x * x * (2 * n - 1) * (2 * n - 1) / (4 * n * n + 2 * n);
}

double recursion_sum(double last, int cur) 
{
    double element = (cur != 1) ? last * get_multiplier(cur - 1) : x;
    if (cur == n) 
    {
        return element;
    } 
    else 
    {
        double sum = element + recursion_sum(element, cur + 1);
        return sum;
    }
}

double get_sum(double argument, int count) 
{
    x = argument;
    n = count;
    return recursion_sum(0, 1);
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