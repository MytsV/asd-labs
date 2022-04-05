#include <stdio.h>
#include <stdbool.h>
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
        bool inFirstRange = x > 2 && x <= 12;
        bool inSecondRange = x > 22 && x < 32;
        if (inFirstRange || inSecondRange) 
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

    return 0;
}