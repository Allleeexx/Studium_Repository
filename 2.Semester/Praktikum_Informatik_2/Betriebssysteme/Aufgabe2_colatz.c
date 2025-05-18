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

int main(){
	
	int ergebnis = collatzfunction(100000);
	printf("Ergebnis: %d\n", ergebnis);

	return 0;
}
