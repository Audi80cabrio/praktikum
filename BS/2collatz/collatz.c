#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> //Für Multithreading
#include <time.h>

#define MAX 100000000 //Obergrenze für Startwerte der Collatz-Folge (also von 1 bis 100 Mio)

typedef struct {
    unsigned int start; //Untere Grenze des Bereichs, den ein Thread untersuchen soll (Startwert inklusive).
    unsigned int end;
    unsigned int max_start; // Rückgabe: Startwert mit längster Folge
    unsigned int max_count; // die Länge (Anzahl Schritte) der längsten gefundenen Collatz-Folge im Bereich.
} CollatzArgs;


//Berechnet, wie viele Schritte nötig sind, um aus x über die Collatz-Regeln zur 1 zu kommen
unsigned int collatz_length(unsigned int x) {
    unsigned int count = 0;
    while (x > 1) {
        if (x % 2 == 0)
            x = x / 2;
        else
            x = 3 * x + 1;
        count++;
    }
    return count; //Anzahl der Schritte
}


//was der thread ausführt
//Diese Funktion wird von einem Thread ausgeführt und sucht im
// zugewiesenen Zahlenbereich nach dem Startwert mit der längsten Collatz-Folge
void* collatz_worker(void* arg) {
    CollatzArgs* args = (CollatzArgs*) arg;
    args->max_count = 0;
    args->max_start = 0;


    //Jeder dieser Werte wird als möglicher Startwert einer Collatz-Folge untersucht.
    for (unsigned int i = args->start; i <= args->end; i++) {
        unsigned int len = collatz_length(i);

        //Vergleich & Speichern des Maximums
        if (len > args->max_count) {
            args->max_count = len;
            args->max_start = i;
        }
    }

    return NULL;
}


//Berechnet die Differenz zwischen zwei Zeitpunkten in Nanosekunden
long time_diff_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

int main(int argc, char* argv[]) {
    int num_threads = (argc >= 2) ? atoi(argv[1]) : 4;

    //Speicher für Threads und Argumente anlegen
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    CollatzArgs* args = malloc(num_threads * sizeof(CollatzArgs));

    unsigned int chunk = MAX / num_threads; //Zerlegt den Gesamtbereich (1 bis MAX) in gleich große Stücke

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);


//Jeder Thread bekommt einen eigenen Start- und Endbereich
//Letzter Thread deckt bis MAX ab, um Rundungsfehler zu vermeiden
    for (int i = 0; i < num_threads; i++) {
        args[i].start = i * chunk + 1;
        args[i].end = (i == num_threads - 1) ? MAX : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, collatz_worker, &args[i]);
    }

    unsigned int global_max_count = 0;
    unsigned int global_max_start = 0;


//pthread_join: wartet auf das Ende jedes Threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        if (args[i].max_count > global_max_count) {
            global_max_count = args[i].max_count;
            global_max_start = args[i].max_start;
        }
    }

    //Hier wird der Endzeitpunkt direkt nach dem letzten Thread gesammelt
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long duration_ns = time_diff_ns(start_time, end_time); //(Sekunden-Differenz * 1.000.000.000) + Nanosekunden-Differenz

    printf("Längste Folge: %u Schritte, Startwert: %u\n", global_max_count, global_max_start);
    printf("Zeit: %.2f Sekunden\n", duration_ns / 1e9);

//Speicher freigeben
    free(threads);
    free(args);
    return 0;
}
