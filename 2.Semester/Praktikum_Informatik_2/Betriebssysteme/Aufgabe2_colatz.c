#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define RANGE_START 1
#define RANGE_END 100000000
#define THREAD_COUNT 16


typedef struct{
	int start;
	int end;
	int maxStartValue;
	int maxIterations;
	struct timespec start_time;
	struct timespec end_time;
	float totalRuntime;
}Rechner;

typedef struct {
 	time_t tv_sec; /* Sekunden */
 	long tv_nsec; /* Nanosekunden */
}timespec;


int collatzfunction(unsigned long long x){
	int cnt = 0;
	while(x>1){
		if(x % 2 == 0){
			x = x/2;
		}else{
			x = 3*x+1;
		}
		cnt = cnt + 1;
	}
	return cnt;
}

void* threadFunction(void * arg){
	Rechner* r = (Rechner*) arg;

	clock_gettime(CLOCK_MONOTONIC, &r->start_time);  // Startzeit

	int maxIterations = 0;
	int maxStartValue = 0;

	for(int i=r->start ; i<=r->end; i++){
		int iterations = collatzfunction((unsigned long long)i);
		if(iterations > maxIterations){
			maxIterations = iterations;
			maxStartValue = i;
		}
	}

	r->maxIterations = maxIterations;
	r->maxStartValue = maxStartValue;

	clock_gettime(CLOCK_MONOTONIC, &r->end_time);  // Endzeit

	r->totalRuntime = (r->start_time.tv_sec - r->start_time.tv_sec) + (r->end_time.tv_nsec - r->end_time.tv_nsec) / 1e9;

	pthread_exit(NULL);
}

int main(){
	
	pthread_t threads[THREAD_COUNT];
	Rechner bereiche[THREAD_COUNT];


	int block_size = RANGE_END / THREAD_COUNT;
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);


	for(int i=0; i<THREAD_COUNT; i++){
		bereiche[i].start = i*block_size +1;
		bereiche[i].end = (i+1) * block_size;

		pthread_create(&threads[i], NULL, threadFunction, &bereiche[i]);
	}	

	//Alle Threads schließen, ansosnten irgnedwelche Werte
	for(int i=0; i<THREAD_COUNT; i++){
		pthread_join(threads[i], NULL);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);


	//Hier der Bereich um ergebnisse zu printen
	for(int i=0; i<THREAD_COUNT; i++){
		printf("Bereich %d: \nStartwert: %d\nEndwert: %d\nMaxIterations: %d\nMaxStartValue: %d\nThreadLaufzeit: %lf Sekunden", i, bereiche[i].start, bereiche[i].end, bereiche[i].maxIterations, bereiche[i].maxStartValue, bereiche[i].totalRuntime);
		printf("\n\n");
	}

	// Globales Maximum finden
	int globalMaxIterations = 0;
	int globalMaxStartValue = 0;

	for(int i = 0; i < THREAD_COUNT; i++){
		if(bereiche[i].maxIterations > globalMaxIterations){
			globalMaxIterations = bereiche[i].maxIterations;
			globalMaxStartValue = bereiche[i].maxStartValue;
		}
	}

	printf("Längste Collatz-Folge im Bereich [%d - %d]:\nStartwert: %d\nAnzahl Schritte: %d\n", RANGE_START, RANGE_END, globalMaxStartValue, globalMaxIterations);


	double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	printf("Bearbeitungszeit: %.6f Sekunden\n", elapsed);

	return 0;
}
