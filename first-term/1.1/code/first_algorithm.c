#include <stdio.h>
#include <stdlib.h>

int main() 
{
    printf("Input X value:");
    double x;
    scanf("%lf", &x);

    double y;
    if (x <= 0) 
    {
        y = - x * x - 12;
        printf("The result Y is: %lf", y);
    } 
    else if (x > 2) 
    {
        if (x <= 12) 
        {
            y = -9 * x * x * x + 5 * x * x;
            printf("The result Y is: %lf", y);
        } 
        else if (x > 22) 
        {
            if (x < 32) 
            {
                y = -9 * x * x * x + 5 * x * x;
                printf("The result Y is: %lf", y);
            } 
            else 
            {
                printf("The input was wrong, it is out of accepted range");
            }
        } 
        else 
        {
            printf("The input was wrong, it is out of accepted range");
        }
    } 
    else 
    {
        printf("The input was wrong, it is out of accepted range");
    }

    return 0;
}