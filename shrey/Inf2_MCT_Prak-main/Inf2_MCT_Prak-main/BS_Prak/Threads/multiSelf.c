#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Ranges
{
    bool notPrime;
    int num;
    int start;
    int end;
} Ranges;

void *primeCalc(void *args)
{
    // uint8_t isPrime = 0;
    Ranges *range = (Ranges *)args;

    for (size_t i = range->start; i <= range->end; i++)
    {
        if (range->num % i == 0)
        {
            range->notPrime = 1;
            break;
        }
        else
        {
            range->notPrime = 0;
        }
    }
    return NULL;
}

int main()
{
    printf("Enter a number: ");
    int num;
    scanf("%d", &num);
    printf("Enter number of threads: ");
    int numThreads;
    scanf("%d", &numThreads);

    int interval = num / numThreads;

    Ranges ranges[numThreads];

    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].num = num;
        ranges[i].start = i * interval + 2;
        ranges[i].end = (i + 1) * (interval + 1);
        if (ranges[i].end >= num && i == numThreads - 1)
        {
            ranges[i].end = num - 1;
        }
    }

    pthread_t threads[numThreads];

    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i], NULL, primeCalc, &ranges[i]);
    }

    bool check = 0;
    for (int i = 0; i < numThreads; i++)
    {
        check = (check || ranges[i].notPrime);
        if (check)
        {
            printf("%d is not prime!\n", num);
            break;
        }
        if (!check && i == numThreads - 1)
        {
            printf("%d is prime!\n", num);
        }
    }
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}