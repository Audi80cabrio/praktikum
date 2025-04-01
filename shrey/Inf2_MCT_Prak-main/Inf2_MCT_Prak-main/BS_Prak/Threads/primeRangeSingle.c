#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define N 1000000

struct timespec start, end;

void *print_primes(void *arg)
{
    bool isPrime[N + 1];

    // Initialize all numbers as prime
    for (int i = 0; i <= N; i++)
    {
        isPrime[i] = true;
    }

    // Mark non-prime numbers
    for (int p = 2; p * p <= N; p++)
    {
        if (isPrime[p] == true)
        {
            for (int i = p * p; i <= N; i += p)
            {
                isPrime[i] = false;
            }
        }
    }

    // Print all prime numbers
    for (int p = 2; p <= N; p++)
    {
        if (isPrime[p])
        {
            // Check if it's the last prime number
            int next_p = p + 1;
            while (next_p <= N && !isPrime[next_p])
            {
                next_p++;
            }
            if (next_p > N)
            {
                printf("%d\n", p); // Print with newline if it's the last prime
            }
            else
            {
                printf("%d, ", p); // Print with comma and space otherwise
            }
        }
    }

    return NULL;
}

int main()
{
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t prime_thread;

    // Create a new thread that will execute 'print_primes'
    pthread_create(&prime_thread, NULL, print_primes, NULL);

    // Wait for the thread to finish
    pthread_join(prime_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Total execution time: %f seconds\n", exec_time);

    return 0;
}