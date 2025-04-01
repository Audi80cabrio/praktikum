#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 1000000
#define THREAD_NUM 2
#define BEREICH 10000

// Array um einzelen Threadzeiten zu speichern
double einzelZeit[THREAD_NUM];

int checkPrime(int x)
{
    if (x <= 1)
    {
        return 0;
    }
    for (int i = 2; i < x; i++)
    {
        if (x % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

void *allPrimes(void *p_threadID)
{

    struct timespec startThread, endeThread;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startThread);

    // ID von dem jeweiligen Thread
    int threadID = (int)p_threadID;

    // Immer wieder die bereiche neu zuordnene
    for (int batch = 0; batch < MAX / (THREAD_NUM * BEREICH); ++batch)
    {
        // Bereich den jeder Thread durchrechnet
        int start = threadID * BEREICH + batch * THREAD_NUM * BEREICH;
        int ende = start + BEREICH;
        // all Primes Func mit individuellem Start/Ende
        for (int i = start; i < ende; i++)
            if (checkPrime(i))
                printf("%d\n", i);
    }

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endeThread);
    einzelZeit[threadID] = (endeThread.tv_sec - startThread.tv_sec) + (endeThread.tv_nsec - startThread.tv_nsec) / 1000000000;

    pthread_exit(NULL);
}

int main()
{
    // Ein array mit den ThreadIDs, um sie zuzuteilen
    pthread_t threads[THREAD_NUM];

    struct timespec start, ende;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Richtige anzahl an Threads erstellen
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create(&threads[i], NULL, allPrimes, (void *)i);

    // Immer warten bis alle threads durch sind
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &ende);
    double absolut = (ende.tv_sec - start.tv_sec) + (ende.tv_nsec - start.tv_nsec) / 1000000000;
    printf("Vergangene Zeit: %.6ld sec\n", absolut);

    for (int i = 0; i < THREAD_NUM; i++)
    {
        printf("Zeit von Thread %d: %.6ld sec\n", i, einzelZeit[i]);
    }

    return 0;
}
