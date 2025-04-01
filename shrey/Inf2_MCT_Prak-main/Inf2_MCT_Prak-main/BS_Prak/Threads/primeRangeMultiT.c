#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// #define MAX 1000000

int cnt = 0;
pthread_mutex_t cnt_lock;

struct timespec start, end;

typedef struct Range
{
    int start;
    int end;
    double exec_time;
} Range;


bool checkPrime(int a)
{
    if (a <= 1)
    {
        return false;
    }

    // int sqrtA = sqrt(a); -> optimization is too fast for run time difference
    // between single and multi-threaded version

    // a/2 can be used for time difference between single and multi-threaded version
    for (int i = 2; i <= a / 2; i++)
    {
        if (a % i == 0)
        {
            return false;
        }
    }

    pthread_mutex_lock(&cnt_lock);
    cnt++;
    pthread_mutex_unlock(&cnt_lock);

    return true;
}

void *print_primes(void *arg)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
    Range *range = (Range *)arg;

    for (int i = range->start; i <= range->end; i++)
    {
        if (checkPrime(i))
        {
            printf("%d\n", i);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    range->exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    return NULL;
}

int main(int argc, char *argv[])
{
    clock_gettime(CLOCK_MONOTONIC, &start); // Start time
    // pthread_mutex_init(&cnt_lock, NULL);

    if (argc != 3)
    {
        printf("Usage: %s (Number of threads) (Max Number)\n", argv[0]);
        return 1;
    }

    int numThreads = atoi(argv[1]);
    int max = atoi(argv[2]);

    pthread_t threads[numThreads];
    Range ranges[numThreads];

    int interval = max / numThreads;

    // calculate the range for each thread and create the threads

    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].start = i * interval + 1;
        ranges[i].end = (i + 1) * interval;
        if (i == numThreads - 1)
            ranges[i].end = max - 1; // Last thread checks up to num - 1
        pthread_create(&threads[i], NULL, print_primes, &ranges[i]);
    }

    // Wait for the thread to finish
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    /*
        printf("Total prime numbers: %d\n", cnt);

        pthread_mutex_destroy(&cnt_lock);
    */

    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time of thread %d: %f seconds\n", i, ranges[i].exec_time);
    }

    printf("Total prime numbers: %d\n", cnt);
    printf("\nThreads: %d, Max Number: %d\n", numThreads, max);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Total execution time: %f seconds\n", exec_time);
    return 0;
}