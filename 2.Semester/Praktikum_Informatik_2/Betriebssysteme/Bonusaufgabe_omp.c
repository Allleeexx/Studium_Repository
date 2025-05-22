#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>


#define RANGE_START 1
#define RANGE_END 100000000



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

void threadFunction(void * arg){
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

	r->totalRuntime = (r->end_time.tv_sec - r->start_time.tv_sec) + (r->end_time.tv_nsec - r->start_time.tv_nsec) / 1e9;
}

int main(){
	int thread_count =  omp_get_max_threads();
	Rechner bereiche[thread_count];


	int block_size = RANGE_END / thread_count;
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	#pragma omp parallel for
	for(int tID = 0; tID < thread_count; tID++){
		bereiche[tID].start = tID*block_size +1;
		bereiche[tID].end = (tID+1) * block_size;
		threadFunction(&bereiche[tID]);

		//Hier der Bereich um ergebnisse zu printen
		#pragma omp critical
		{
			printf("Bereich %d: \nStartwert: %d\nEndwert: %d\nMaxIterations: %d\nMaxStartValue: %d\nThreadLaufzeit: %lf Sekunden", tID, bereiche[tID].start, bereiche[tID].end, bereiche[tID].maxIterations, bereiche[tID].maxStartValue, bereiche[tID].totalRuntime);			
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);




	// Globales Maximum finden
	int globalMaxIterations = 0;
	int globalMaxStartValue = 0;

	#pragma omp critical
	{
		int tID = omp_get_thread_num();
		if(bereiche[tID].maxIterations > globalMaxIterations){
			globalMaxIterations = bereiche[tID].maxIterations;
			globalMaxStartValue = bereiche[tID].maxStartValue;
		}
	}


	printf("Längste Collatz-Folge im Bereich [%d - %d]:\nStartwert: %d\nAnzahl Schritte: %d\n", RANGE_START, RANGE_END, globalMaxStartValue, globalMaxIterations);


	double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	printf("Bearbeitungszeit: %.6f Sekunden\n", elapsed);

	return 0;
}
