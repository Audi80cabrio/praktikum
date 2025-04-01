#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct
{
    bool isPrime;
    int start;
    int end;
    int num;
} Range;

void *primeCalc(void *arg)
{

    Range *range = (Range *)arg;
    for (int i = range->start; i <= range->end; i++)
    {
        if (range->num % i == 0)
        {
            range->isPrime = 0;
            break;
        }
        else
        {
            range->isPrime = 1;
        }
    }
    return NULL;
}

int main()
{
    int num, numThreads;
    printf("Enter a number: ");
    scanf("%d", &num);
    printf("Enter number of threads: ");
    scanf("%d", &numThreads);

    pthread_t threads[numThreads];
    Range ranges[numThreads];

    int interval = num / numThreads;
    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].start = i * interval + 2;
        ranges[i].end = (i + 1) * interval;
        ranges[i].num = num;
        if (i == numThreads - 1)
            ranges[i].end = num - 1; // Last thread checks up to num - 1
        pthread_create(&threads[i], NULL, primeCalc, &ranges[i]);
    }

    bool finalCheck;
    for (int i = 0; i < numThreads; i++)
    {
        bool check = ranges[i].isPrime;
        if (check == 1 && i != 0)
        {
            check = (check && ranges[i - 1].isPrime);
        }
        finalCheck = check;
    }
    if (finalCheck == 1)
    {
        printf("%d is prime!\n", num);
    }
    else
    {
        printf("%d is not prime!\n", num);
    }

    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}