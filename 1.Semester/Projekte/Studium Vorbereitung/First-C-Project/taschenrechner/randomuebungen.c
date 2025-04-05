#include <stdio.h>

void bubblesort(){
    int uebergabewert1, uebergabewert2;
    int grades[] = {3,1, 62, 42, 8765, 23, 83, 12};
    int gradeslength = sizeof(grades) / sizeof(grades[0]);

    for(int j=0; j<gradeslength; j++){
        for(int i=0; i<gradeslength; i++){
            if(grades[i]> grades[i+1]){
                uebergabewert1 = grades[i];
                grades[i] = grades[i+1];
                grades[i+1] = uebergabewert1;
            }
        }
    }
    for(int x=0; x<gradeslength; x++){
        printf("%d\n", grades[x]);
    }
}


void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void uebung1() {
    int grades[] = {3, 1, 62, 42, 8765, 23, 83, 12};
    int gradeslength = sizeof(grades) / sizeof(grades[0]);

    for (int i = 0; i < gradeslength - 1; i++) {
        if (grades[i] > grades[i + 1]) {
            // Tausche die Elemente, falls sie in der falschen Reihenfolge sind
            swap(&grades[i], &grades[i + 1]);
        }
    }

    // Ausgabe des sortierten Arrays
    for (int i = 0; i < gradeslength; i++) {
        printf("%d ", grades[i]);
    }
    printf("\n");
}

void Stringumgang(){
    char vorname[50];
    char nachname [50];

    printf("Bitte gebe deinen Vornamen ein\n");
    scanf("%s", vorname);
    printf("Bitte gebe deinen Nachnamen ein\n");
    scanf("%s", nachname);


    printf("Vorname: %s\n", vorname);
    printf("Nachname: %s\n", nachname);

    if (strcmp(vorname, nachname) != 0){
        printf("Willkommen %s %s \n", vorname, nachname);
    }else{
        printf("Ich glaube du hast einen Fehler gemacht \n");
    }
}

