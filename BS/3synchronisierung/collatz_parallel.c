
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define MAX_START 100000000

// Globale Variablen
unsigned int current = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
atomic_uint atomic_current = 1;

unsigned long long collatz_sum_mutex = 0;
unsigned long long collatz_sum_atomic = 0;

// Für unsynchronisierte Variante
unsigned int current_unsync = 1;
unsigned long long collatz_sum_unsync = 0;

unsigned int NUM_THREADS = 4; // wird zur laufzeit überschrieben

// Collatz-Längenberechnung
unsigned int collatz_length(unsigned long n) {
    unsigned int count = 0;
    while (n != 1) {
        if (n % 2 == 0) n = n / 2;
        else n = 3 * n + 1;
        count++;
    }
    return count;
}

// Mutex-Version
void* collatz_worker_mutex(void* arg) {
    while (1) {
        unsigned int start;
        pthread_mutex_lock(&mutex);
        if (current > MAX_START) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        start = current++;
        pthread_mutex_unlock(&mutex);
        unsigned int len = collatz_length(start);
        atomic_fetch_add(&collatz_sum_mutex, len);
    }
    return NULL;
}

// Atomare Version
void* collatz_worker_atomic(void* arg) {
    while (1) {
        unsigned int start = atomic_fetch_add(&atomic_current, 1);
        if (start > MAX_START) break;
        unsigned int len = collatz_length(start);
        atomic_fetch_add(&collatz_sum_atomic, len);
    }
    return NULL;
}

// Unsynchronisierte Version
void* collatz_worker_unsync(void* arg) {
    while (1) {
        if (current_unsync > MAX_START) break;
        unsigned int start = current_unsync++; // kein Schutz!
        unsigned int len = collatz_length(start);
        collatz_sum_unsync += len; // kein Schutz!
    }
    return NULL;
}

// Zeitdifferenz
long time_diff_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        NUM_THREADS = atoi(argv[1]);
        if (NUM_THREADS < 1 || NUM_THREADS > 256) {
            printf("Ungültige Threadanzahl: %s\n", argv[1]);
            return 1;
        }
    }

    pthread_t threads[NUM_THREADS];
    struct timespec start_time, end_time;

    printf("Starte mit %u Threads ...\n", NUM_THREADS);

    // --------- MUTEX Variante ---------
    current = 1;
    collatz_sum_mutex = 0;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, collatz_worker_mutex, NULL);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long duration_mutex = time_diff_ns(start_time, end_time);
    printf("Mutex:   Summe = %llu, Zeit = %ld ms\n", collatz_sum_mutex, duration_mutex / 1000000);

    // --------- ATOMIC Variante ---------
    atomic_store(&atomic_current, 1);
    collatz_sum_atomic = 0;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, collatz_worker_atomic, NULL);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long duration_atomic = time_diff_ns(start_time, end_time);
    printf("Atomic:  Summe = %llu, Zeit = %ld ms\n", collatz_sum_atomic, duration_atomic / 1000000);

    // --------- UNSYNCHRONISIERTE Variante ---------
    current_unsync = 1;
    collatz_sum_unsync = 0;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, collatz_worker_unsync, NULL);
    for (unsigned int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long duration_unsync = time_diff_ns(start_time, end_time);
    printf("Unsync:  Summe = %llu, Zeit = %ld ms\n", collatz_sum_unsync, duration_unsync / 1000000);

    return 0;
}
