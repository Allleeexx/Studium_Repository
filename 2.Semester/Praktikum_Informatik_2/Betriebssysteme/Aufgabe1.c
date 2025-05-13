#include <stdio.h>
#include <pthread.h>
#include <string.h>

typedef struct{
	char name [100];
	int length;	
}Student;


void *example_fct(void *args){
	Student *student = (Student*) args;
	student->length = strlen(student->name);
	printf("Name in example: %s\n", student->name);
	printf("Length von Name: %d\n\n", student->length);
	return NULL;
}



int main(){
	Student student;	
	strncpy(student.name, "Alex", 100);
	pthread_t threadA/*, threadB*/;
	pthread_create(&threadA, NULL, &example_fct, &student);
	//pthread_create(&threadB, NULL, &example_fct, NULL);
	
	pthread_join(threadA, NULL);
//	pthread_join(threadB, NULL);

	printf("Name in main: %s\n", student.name);
	printf("Length in main: %d\n", student.length);
}
