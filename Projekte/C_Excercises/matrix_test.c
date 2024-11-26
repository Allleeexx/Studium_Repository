#include <stdio.h>
#define Z 3
#define S 3

int a [Z][S] = {
    {2,1,-2},
    {3,4,1},
    {6,-5,-3},
};

int b [Z][S] = {
    {6,1,2},
    {-1,4,3},
    {-2,-4,5},
};

int matrix_multiplication(int array_a[][S], int array_b[][S]);

int matrix_multiplication(int array_a[][S], int array_b[][S]){
    int zwischenergebnis1,zwischenergebnis2 = 0;
    int ergebnis [Z][S] = {{0,0, 0}};
    for(int i=0; i<Z; i++){
        for(int j=0; j<S; j++){
            for(int k=0; k<S; k++){
                ergebnis[i][j] +=  array_a[i][k]*array_b[k][j];
            }
        }
    }

    for(int x=0; x<Z; x++){
        for(int z=0; z<S; z++){
            printf("Zeile %d und Spalte %d: \t %d\n", x,z, ergebnis[x][z]);
        }
    }
}

int main(){
    matrix_multiplication(a, b);
    return 0;
}