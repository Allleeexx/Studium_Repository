#include <stdio.h>
#include <pthread.h>
#include <string.h>


#define RANGE_START 1;
#define RANGE_END 100000000;

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


void *example_fct(void *args){
	Student *student = (Student*) args;
	student->length = strlen(student->name);
	printf("Name in example: %s\n", student->name);
	printf("Length von Name: %d\n\n", student->length);
	return NULL;
}

int collatzfunction(int input){
	int cnt = 0;
	whiel(x>1){
		x = (x%2==0)? x/2:3*x+1;
		cnt++;
	}
	return cnt;
}

int main(){
	Student student;
	Rechner rechner;

	
	strncpy(student.name, "Alex", 100);
	pthread_t threadA/*, threadB*/;


	//Hier im Bereich Thread Funktion aufrufen
	pthread_create(&threadA, NULL, &example_fct, &student);
	pthread_create(&threadA, NULL, &collatzfunction, &rechner);	

	//In dem Bereich Thread schließen
	pthread_join(threadA, NULL);


	//////////////////////////////////////////////////////
}
