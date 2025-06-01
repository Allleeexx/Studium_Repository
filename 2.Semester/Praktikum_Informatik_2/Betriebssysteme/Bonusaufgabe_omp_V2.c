//Um omp zu compilieren mit gcc -fopenmp -o Name Name.c

#include <stdio.h>
#include <time.h>
#include <omp.h>

#define RANGE_START 1
#define RANGE_END 100000000

int collatzfunction(unsigned long long x) {
    int cnt = 0;
    while(x > 1) {
        if(x % 2 == 0) {
            x = x / 2;
        } else {
            x = 3 * x + 1;
        }
        cnt++;
    }
    return cnt;
}

int main() {
    int globalMaxIterations = 0;
    int globalMaxStartValue = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    #pragma omp parallel
    {
        int localMaxIterations = 0;
        int localMaxStartValue = 0;

        #pragma omp for nowait
        for (int i = RANGE_START; i <= RANGE_END; i++) {
            int iterations = collatzfunction((unsigned long long)i);
            if (iterations > localMaxIterations) {
                localMaxIterations = iterations;
                localMaxStartValue = i;
            }
        }

        #pragma omp critical
        {
            if (localMaxIterations > globalMaxIterations) {
                globalMaxIterations = localMaxIterations;
                globalMaxStartValue = localMaxStartValue;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Längste Collatz-Folge im Bereich [%d - %d]:\nStartwert: %d\nAnzahl Schritte: %d\n", RANGE_START, RANGE_END, globalMaxStartValue, globalMaxIterations);
    printf("Bearbeitungszeit: %.6f Sekunden\n", elapsed);

    return 0;
}
