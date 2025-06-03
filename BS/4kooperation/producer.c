#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <unistd.h>

#define NUM_PRODUCERS 50
#define NUM_CONSUMERS 30
#define ITEMS_PER_PRODUCER 10000
#define MAX_PRODUCED (NUM_PRODUCERS * ITEMS_PER_PRODUCER)
#define MAX_LIST_LENGTH 5

// === Verkettete Liste ===
typedef struct Node {
    int payload;
    struct Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;
int list_length = 0;  // Nicht atomar – wird im mutex geschützt

// === Synchronisation & Zähler ===
sem_t sem_items;
sem_t sem_slots;
sem_t sem_mutex;

atomic_int gl_prod = 0;
atomic_int gl_cons = 0;
atomic_int produced_count = 0;

// === Hilfsfunktionen ===
void enqueue(int value) {
    Node* new_node = malloc(sizeof(Node));
    new_node->payload = value;
    new_node->next = NULL;

    if (tail == NULL) {
        head = tail = new_node;
    } else {
        tail->next = new_node;
        tail = new_node;
    }
}

int dequeue() {
    if (head == NULL) return -1;
    Node* tmp = head;
    int value = tmp->payload;
    head = head->next;
    if (head == NULL) tail = NULL;
    free(tmp);
    return value;
}

// === Producer ===
void* producer(void* arg) {
    unsigned int seed = (unsigned int)pthread_self();

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int num = rand_r(&seed) % 1000;

        sem_wait(&sem_slots);
        sem_wait(&sem_mutex);

        enqueue(num);
        list_length++;
        if (list_length > MAX_LIST_LENGTH) {
            fprintf(stderr, "Fehler: Listenlänge überschreitet %d\n", MAX_LIST_LENGTH);
        }

        atomic_fetch_add(&gl_prod, num);
        atomic_fetch_add(&produced_count, 1);

        sem_post(&sem_mutex);
        sem_post(&sem_items);
    }

    // Nur ein Producer schickt Endemarker
    static atomic_int done = 0;
    if (atomic_fetch_add(&done, 1) == NUM_PRODUCERS - 1) {
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            sem_wait(&sem_slots);
            sem_wait(&sem_mutex);
            enqueue(-1);
            list_length++;
            sem_post(&sem_mutex);
            sem_post(&sem_items);
        }
    }

    return NULL;
}

// === Consumer ===
void* consumer(void* arg) {
    while (1) {
        sem_wait(&sem_items);
        sem_wait(&sem_mutex);

        int value = dequeue();
        list_length--;

        if (value == -1) {
            sem_post(&sem_mutex);
            sem_post(&sem_items);
            sem_post(&sem_slots);
            break;
        }

        atomic_fetch_add(&gl_cons, value);

        sem_post(&sem_mutex);
        sem_post(&sem_slots);
    }

    return NULL;
}

// === Main ===
int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_items, 0, 0);
    sem_init(&sem_slots, 0, MAX_LIST_LENGTH); // exakt 5 Slots erlaubt

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_create(&producers[i], NULL, producer, NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer, NULL);

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    sem_destroy(&sem_mutex);
    sem_destroy(&sem_items);
    sem_destroy(&sem_slots);

    while (head != NULL) dequeue();

    printf("ALLE THREADS BEENDET\n");
    printf("Produziert (Summe): %d\n", gl_prod);
    printf("Konsumiert (Summe): %d\n", gl_cons);
    return 0;
}
