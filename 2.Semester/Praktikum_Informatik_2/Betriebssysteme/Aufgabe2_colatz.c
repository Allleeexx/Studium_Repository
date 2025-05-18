#include <stdio.h>
#include <pthread.h>
#include <string.h>


#define RANGE_START 1
#define RANGE_END 100000000
#define THREAD_COUNT 5

typedef struct{
	char name [100];
	int length;	
}Student;

typedef struct{
	int start;
	int end;
	int maxStartValue;
	int maxIterations;
}Rechner;



int collatzfunction(int x){
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

	int maxIterations = 0;
	int maxStartValue = 0;

	for(int i=r->start ; i<=r->end; i++){
		int iterations = collatzfunction(i);
		if(iterations > maxIterations){
			maxIterations = iterations;
			maxStartValue = i;
		}
	}

	r->maxIterations = maxIterations;
	r->maxStartValue = maxStartValue;

	pthread_exit(NULL);
}

int main(){
	
	pthread_t threads[THREAD_COUNT];
	Rechner bereiche[THREAD_COUNT];


	int block_size = RANGE_END / THREAD_COUNT;
	
	for(int i=0; i<THREAD_COUNT; i++){
		bereiche[i].start = i*block_size +1;
		bereiche[i].end = (i+1) * block_size;

		pthread_create(&threads[i], NULL, threadFunction, &bereiche[i]);
	}	


	//Hier der Bereich um ergebnisse zu printen
	for(int i=0; i<THREAD_COUNT; i++){
		printf("Bereich %d: \nStartwert: %d\nEndwert: %d\nMaxIterations: %d\nMaxStartValue: %d\n", i, bereiche[i].start, bereiche[i].end, bereiche[i].maxIterations, bereiche[i].maxStartValue);
	}
	return 0;
}
