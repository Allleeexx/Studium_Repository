#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <stdlib.h>

#define PRODUCE_COUNT 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2


//------------------Structs-----------------------//

typedef struct node{
	struct node* next;
	int value;
}Node;

//------------------Global Vars-----------------------//

Node* head = NULL;
sem_t sem_items;
sem_t sem_slots;
sem_t sem_mutex;
int active_producers = NUM_PRODUCERS;

//------------------Functions-----------------------//

int get_random(){
	return rand() % 100+1;
}

void add_to_list(int value){
	Node* new_node = (Node*)malloc(sizeof(Node));
	new_node->value = value;
	new_node->next = head;
	head = new_node;
}

int remove_from_list(){
	if(head == NULL){
		return -1;
	}

	Node* current = head;
	Node* previous = NULL;

	while(current->next != NULL){
		previous = current;
		current = current->next;
	}

	int value = current->value;

	if(previous == NULL){
		head = NULL;
	}else{
		previous->next = NULL;
	}

	free(current);
	return value;
}


//Producer Thread
void* producer(void* arg){
	unsigned int seed = (unsigned int)pthread_self();
	srand(seed);

	for(int i=0; i<PRODUCE_COUNT; i++){
		int value = get_random();
		sleep(rand()%2);

		sem_wait(&sem_mutex);
		add_to_list(value);
		sem_post(&sem_mutex);
		sem_post(&sem_items);
		printf("[Producer %lu] Produziert: %d\n", pthread_self(), value);
	}

	sem_wait(&sem_mutex);
	active_producers--;
	sem_post(&sem_mutex);

	pthread_exit(NULL);
}

//Consumer Thread
void* consumer(void* arg){
	unsigned int seed = (unsigned int)pthread_self();
    srand(seed);

    while (1) {
        sem_wait(&sem_items);  // Warten auf ein Item

        sem_wait(&sem_mutex);
        if (head == NULL && active_producers == 0) {
            sem_post(&sem_mutex);
            sem_post(&sem_items);  // Für andere Consumer
            break;
        }

        if (head != NULL) {
            int value = remove_from_list();
            sem_post(&sem_mutex);

            collatz(value);  // Simuliere Arbeit
            printf("[Consumer %lu] Verarbeitet: %d\n", pthread_self(), value);
            sleep(rand() % 2);
        } else {
            sem_post(&sem_mutex);
            sem_post(&sem_items);  // Für andere Consumer
        }
    }

    pthread_exit(NULL);
}

int main(){
	pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    sem_init(&sem_items, 0, 0);
    sem_init(&sem_mutex, 0, 1);

    // Threads erzeugen
    for (int i = 0; i < NUM_PRODUCERS; ++i)
        pthread_create(&producers[i], NULL, producer, NULL);

    for (int i = 0; i < NUM_CONSUMERS; ++i)
        pthread_create(&consumers[i], NULL, consumer, NULL);

    // Auf Threads warten
    for (int i = 0; i < NUM_PRODUCERS; ++i)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; ++i)
        pthread_join(consumers[i], NULL);

    // Aufräumen
    sem_destroy(&sem_items);
    sem_destroy(&sem_mutex);

    printf("Alle Threads beendet. Programm fertig.\n");

	return 0;
}
