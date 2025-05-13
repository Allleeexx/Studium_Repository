#include <stdio.h>
#include <pthread.h>

void *example_fct(void *args){
	size_t i;
	struct timespec sleep = {0, 1000};

	for(i =0; i<100; i++){
		putchar('a' +  pthread_self() % 26);
		nanosleep(&sleep, NULL);
	}
	return NULL;
}

int main(){
	pthread_t threadA, threadB;
	pthread_create(&threadA, NULL, &example_fct, NULL);
	pthread_create(&threadB, NULL, &example_fct, NULL);

	pthread_join(threadA, NULL);
	pthread_join(threadB, NULL);
}
