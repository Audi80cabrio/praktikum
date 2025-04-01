#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

int current_number = 2; // Start from the first prime number
int max_number;
pthread_mutex_t number_lock;

int cnt = 0;
pthread_mutex_t cnt_lock;

typedef struct Range
{
    double exec_time;
} Range;

bool checkPrime(int a)
{
    if (a <= 1)
    {
        return false;
    }

    for (int i = 2; i <= a - 1; i++)
    {
        if (a % i == 0)
        {
            return false;
        }
    }

    return true;
}

void *print_primes(void *arg)
{
    Range *range = (Range *)arg;
    struct timespec start, end;

    while (1)
    {
        pthread_mutex_lock(&number_lock);

        if (current_number >= max_number)
        {
            pthread_mutex_unlock(&number_lock);
            return NULL;
        }

        int number = current_number;
        current_number++;

        pthread_mutex_unlock(&number_lock);

        clock_gettime(CLOCK_MONOTONIC, &start);

        if (checkPrime(number))
        {
            printf("%d\n", number);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        range->exec_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s (Number of threads) (Max Number)\n", argv[0]);
        return 1;
    }

    int numThreads = atoi(argv[1]);
    max_number = atoi(argv[2]);

    pthread_t threads[numThreads];
    Range ranges[numThreads];

    pthread_mutex_init(&number_lock, NULL);

    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].exec_time = 0.0; // Initialize exec_time to 0
        pthread_create(&threads[i], NULL, print_primes, &ranges[i]);
    }

    pthread_mutex_destroy(&number_lock);

    // First loop to join all threads
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Second loop to print the execution times
    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time of thread %d: %f seconds\n", i, ranges[i].exec_time);
    }
    

    return 0;
}