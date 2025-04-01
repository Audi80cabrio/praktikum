#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>

void *primeCalc(void *num)
{
    int *val = (int *)num;
    // uint8_t isPrime = 0;
    for (size_t i = 2; i < *val - 1; i++)
    {
        if (*val % i == 0)
        {
            printf("%d is not prime!\n", *val);
            break;
        }
        else
            printf("%d is prime!\n", *val);
        break;
    }
    return NULL;
}

int main()
{
    int num = 17;
    pthread_t thread1;
    pthread_create(&thread1, NULL, primeCalc, &num);
    pthread_join(thread1, NULL);

    return 0;
}