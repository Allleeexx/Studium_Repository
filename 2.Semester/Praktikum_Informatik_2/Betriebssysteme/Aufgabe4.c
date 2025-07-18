#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdatomic.h>

#define PRODUCE_COUNT 10000        //Jeder producer produziert so viele Werte mal 
#define NUM_PRODUCERS 50         //anazahl an producern
#define NUM_CONSUMERS 30         //anzahl consumer

typedef struct node {
    struct node* next;
    int value;
} Node;

Node* head = NULL;
sem_t sem_items;
sem_t sem_mutex;
sem_t sem_free;
int active_producers = NUM_PRODUCERS;
atomic_int werte_producers = 0;
atomic_int werte_consumers = 0;
atomic_int list_length = 0;


// Collatz-Funktion (Dummy-Arbeit)
void collatz(int n) {
    while (n != 1) {
        if (n % 2 == 0)
            n = n / 2;
        else
            n = 3 * n + 1;
    }
}

int get_random() {
    return rand() % 100 + 1;
}

void add_to_list(int value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->value = value;
    new_node->next = head;
    head = new_node;

	 int len = atomic_fetch_add(&list_length, 1);
	 if(len>5){
		printf("Fehler: Mehr als 5 Elemente in Liste -> Aktuelle Laenge: %d\n", len);
	}
}

int remove_from_list() {
    if (head == NULL) {
        return -1;
    }

    Node* current = head;
    Node* previous = NULL;

    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }

    int value = current->value;

    if (previous == NULL) {
        head = NULL;
    } else {
        previous->next = NULL;
    }

    free(current);
    atomic_fetch_sub(&list_length, 1);
    return value;
}

void* producer(void* arg) {
    unsigned int seed = (unsigned int)pthread_self();
    srand(seed);

    for (int i = 0; i < PRODUCE_COUNT; i++) {
        int value = get_random();
        //sleep(rand() % 2);

        sem_wait(&sem_free);

        sem_wait(&sem_mutex);
        add_to_list(value);
        atomic_fetch_add(&werte_producers, value);
        sem_post(&sem_mutex);
        sem_post(&sem_items);

//        printf("[Producer %lu] Produziert: %d\n", pthread_self(), value);
    }

    sem_wait(&sem_mutex);
    active_producers--;
    sem_post(&sem_mutex);

    pthread_exit(NULL);
}

void* consumer(void* arg) {
    unsigned int seed = (unsigned int)pthread_self();
    srand(seed);

    while (1) {
        sem_wait(&sem_items);  // Warten auf ein Item

        sem_wait(&sem_mutex);
        if (head == NULL && active_producers == 0) {
            sem_post(&sem_mutex);
            break;
        }

        if (head != NULL) {
            int value = remove_from_list();
            sem_post(&sem_mutex);
            sem_post(&sem_free);

            collatz(value);  // Simuliere Arbeit
            atomic_fetch_add(&werte_consumers, value);
  //          printf("[Consumer %lu] Verarbeitet: %d\n", pthread_self(), value);
         //   sleep(rand() % 2);
        } else {
            sem_post(&sem_mutex);
        }
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    sem_init(&sem_items, 0, 0);
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_free, 0, 5 );

    for (int i = 0; i < NUM_PRODUCERS; ++i)
        pthread_create(&producers[i], NULL, producer, NULL);

    for (int i = 0; i < NUM_CONSUMERS; ++i)
        pthread_create(&consumers[i], NULL, consumer, NULL);

    for (int i = 0; i < NUM_PRODUCERS; ++i)
        pthread_join(producers[i], NULL);

    // Signal an Consumer, dass keine neuen Items mehr kommen
    for (int i = 0; i < NUM_CONSUMERS; i++)
        sem_post(&sem_items);

    for (int i = 0; i < NUM_CONSUMERS; ++i)
        pthread_join(consumers[i], NULL);

    sem_destroy(&sem_items);
    sem_destroy(&sem_mutex);

    // Aufräumen der Liste
    while (head != NULL) {
        Node* temp = head;
        head = head->next;
        free(temp);
    }

    printf("\nAlle Threads beendet. Programm fertig.\n");
    printf("Summe produziert: %d\n", atomic_load(&werte_producers));
    printf("Summe consumiert: %d\n", atomic_load(&werte_consumers));

    return 0;
}
