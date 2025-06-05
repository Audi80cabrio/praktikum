//gcc -fopenmp -O2 collatz_openmp.c -o collatz_openmp

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// collatz länge
unsigned int collatz_length(unsigned long n) {
    unsigned int length = 1;
    while (n != 1) {
        if (n % 2 == 0)
            n = n / 2;
        else
            n = 3 * n + 1;
        length++;
    }
    return length;
}

int main() {
    const unsigned int MAX_START = 100000000; //bis 100.000.000
    unsigned int max_length = 0;
    unsigned int max_start = 0;
    int nThreads = 0;

    double start_time = omp_get_wtime();        //getter für aktuelle zeit in sekunden(double)

    // Paralleler Bereich mit OpenMP
    #pragma omp parallel        //öffnet parallelen bereich
    {
        unsigned int local_max_length = 0;
        unsigned int local_max_start = 0;

        #pragma omp master      //nur der master führt das aus
        {
            nThreads = omp_get_num_threads();       //getter für aktuell aktive threads(aus der anleitung)
            printf("Anzahl der OMP-Threads: %d\n", nThreads);
        }

        // Parallele for-Schleife
        #pragma omp for     //gleichmäßiges zerteilen der for schleife auf threds
        for (unsigned int i = 1; i <= MAX_START; i++) {
            unsigned int len = collatz_length(i);
            if (len > local_max_length) {
                local_max_length = len;
                local_max_start = i;
            }
        }

        // Kritischer Abschnitt: Zusammenführen der Ergebnisse
        #pragma omp critical        //schützt das updaten der globalen maximalwerte ähnlich wie mutex
        {
            if (local_max_length > max_length) {
                max_length = local_max_length;
                max_start = local_max_start;
            }
        }
    }

    double end_time = omp_get_wtime();      //getter für aktuelle zeit in sekunden(double)
    printf("Längste Folge: %u Schritte, Startwert: %u\n", max_length, max_start);
    printf("Laufzeit: %.2f Sekunden\n", end_time - start_time);
    return 0;
}
