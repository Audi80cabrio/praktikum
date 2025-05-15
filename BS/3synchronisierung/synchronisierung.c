#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define MAX_CNT 10000000
#define STARTWERT 1

long global_var = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
atomic_int atomic_var = 0;

// ----------- Primitive Ink./Dek. -----------
void* inc_unsync(void* arg) {
    for (size_t i = 0; i < MAX_CNT; ++i) {
        global_var++;
    }
    return NULL;
}

void* dec_unsync(void* arg) {
    for (size_t i = 0; i < MAX_CNT; ++i) {
        global_var--;
    }
    return NULL;
}

// ----------- Mutex Ink./Dek. -----------
void* inc_mutex(void* arg) {
    for (size_t i = 0; i < MAX_CNT; ++i) {
        pthread_mutex_lock(&mutex);
        global_var++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* dec_mutex(void* arg) {
    for (size_t i = 0; i < MAX_CNT; ++i) {
        pthread_mutex_lock(&mutex);
        global_var--;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// ----------- Atomare Collatz-Simulation -----------
void* collatz_atomic(void* arg) {
    while (1) {
        int n = atomic_fetch_add(&atomic_var, 1);
        if (n > MAX_CNT) break;

        int cnt = 0;
        int x = n;
        while (x > 1) {
            if (x % 2 == 0)
                x /= 2;
            else
                x = 3 * x + 1;
            cnt++;
        }
    }
    return NULL;
}

// ----------- Zeitmessung -----------
double measure(void* (*func)(void*), int num_threads) {
    pthread_t threads[num_threads];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; ++i)
        pthread_create(&threads[i], NULL, func, NULL);
    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// ----------- Main für alle Varianten -----------
int main() {
    printf("=== Primitive Version ===\n");
    global_var = 0;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, inc_unsync, NULL);
    pthread_create(&t2, NULL, dec_unsync, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Result without sync: %ld (should be 0)\n\n", global_var);

    printf("=== Mit Mutex ===\n");
    global_var = 0;
    pthread_create(&t1, NULL, inc_mutex, NULL);
    pthread_create(&t2, NULL, dec_mutex, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Result with mutex:   %ld\n\n", global_var);

    printf("=== Collatz mit atomar ===\n");
    for (int threads = 2; threads <= 16; threads *= 2) {
        atomic_var = STARTWERT;
        double t = measure(collatz_atomic, threads);
        printf("Threads: %2d | Zeit: %.4fs\n", threads, t);
    }

    return 0;
}
