#include <stdio.h>

int main(){
	char name[] = "Alex";
	int day = 25;
	int month = 06;
	int year = 2002; 

	printf("Mein Name ist %s\n", name);
	printf("Mein Geburtsdatum ist der %02d.%02d.%04d\n", day, month, year);
	return 0;
}

