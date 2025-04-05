#include <stdio.h>
#define Z 3
#define S 3

int a [Z][S] = {
    {2,-3,1},
    {7,-2,0},
    {3,5,2},
};

int b [Z][S] = {
    {4,2,9},
    {-3,-2,-6},
    {3,6,4},
};

int matrix_addition(int array_a[][S], int array_b[][S]);

int matrix_addition(int array_a[][S], int array_b[][S]){
    int ergebnis [Z][S] = {{0,0, 0}};
    for(int i=0; i<Z; i++){
        for(int j=0; j<S; j++){
            ergebnis[i][j] = array_a[i][j] + array_b[i][j];
        }
    }

    for(int x=0; x<Z; x++){
        for(int z=0; z<S; z++){
            printf("Zeile %d und Spalte %d: \t %d\n", x,z, ergebnis[x][z]);
        }
    }
}

int main(){
    matrix_addition(a, b);
    return 0;
}