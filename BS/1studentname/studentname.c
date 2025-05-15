#include <stdio.h>
#include <string.h>
#include <pthread.h>

// Struktur zur Übergabe an den Thread
struct Student {
    char name[100];      // fester Puffer für Namen
    size_t length;       // Länge des Namens
};

// Thread-Funktion
void* calculate_length(void* arg) {
    struct Student* student = (struct Student*) arg;
    student->length = strlen(student->name); // Länge berechnen
    return NULL; // thread beenden
}

int main() {
    pthread_t thread;
    struct Student s;

    // Name initial setzen
    strncpy(s.name, "Jonas Barnert", sizeof(s.name));
    s.name[sizeof(s.name) - 1] = '\0'; // Sicherheit: Nullterminator setzen

    // Thread starten
    if (pthread_create(&thread, NULL, calculate_length, (void*)&s) != 0) {
        perror("Fehler beim Erstellen des Threads");
        return 1;
    }

    // Auf Beendigung des Threads warten
    if (pthread_join(thread, NULL) != 0) {
        perror("Fehler beim Warten auf den Thread");
        return 1;
    }

    // Ergebnis ausgeben
    printf("Name: %s\n", s.name);
    printf("Länge: %zu\n", s.length);

    return 0;
}
