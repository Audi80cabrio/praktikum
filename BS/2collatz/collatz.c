#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 100000000

typedef struct {
    unsigned int start;
    unsigned int end;
    unsigned int max_start; // Rückgabe: Startwert mit längster Folge
    unsigned int max_count; // Rückgabe: Maximale Länge
} CollatzArgs;

unsigned int collatz_length(unsigned int x) {
    unsigned int count = 0;
    while (x > 1) {
        if (x % 2 == 0)
            x = x / 2;
        else
            x = 3 * x + 1;
        count++;
    }
    return count;
}

void* collatz_worker(void* arg) {
    CollatzArgs* args = (CollatzArgs*) arg;
    args->max_count = 0;
    args->max_start = 0;

    for (unsigned int i = args->start; i <= args->end; i++) {
        unsigned int len = collatz_length(i);
        if (len > args->max_count) {
            args->max_count = len;
            args->max_start = i;
        }
    }

    return NULL;
}

long time_diff_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

int main(int argc, char* argv[]) {
    int num_threads = (argc >= 2) ? atoi(argv[1]) : 4;
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    CollatzArgs* args = malloc(num_threads * sizeof(CollatzArgs));

    unsigned int chunk = MAX / num_threads;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; i++) {
        args[i].start = i * chunk + 1;
        args[i].end = (i == num_threads - 1) ? MAX : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, collatz_worker, &args[i]);
    }

    unsigned int global_max_count = 0;
    unsigned int global_max_start = 0;

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        if (args[i].max_count > global_max_count) {
            global_max_count = args[i].max_count;
            global_max_start = args[i].max_start;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long duration_ns = time_diff_ns(start_time, end_time);

    printf("Längste Folge: %u Schritte, Startwert: %u\n", global_max_count, global_max_start);
    printf("Zeit: %.2f Sekunden\n", duration_ns / 1e9);

    free(threads);
    free(args);
    return 0;
}
