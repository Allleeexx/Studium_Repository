#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    for (int i = 1; i <= 3; i++)
    {
        pid = fork();

        if (pid == 0)
        {
            /*
             * Sohnprozess
             */
            printf("Sohnprozess %d gestartet: UNIX-PID = %d\n", i, getpid());

            for (int j = 1; j <= 3; j++)
            {
                printf("Prozess %d betritt kritischen Bereich\n", i);
                sleep(1);
                printf("Prozess %d verlaesst kritischen Bereich\n", i);

                printf("Prozess %d betritt unkritischen Bereich\n", i);
                sleep(1);
                printf("Prozess %d verlaesst unkritischen Bereich\n", i);
            }

            exit(0);
        }
        else if (pid == -1)
        {
            /*
             * Fehlerfall
             */
            perror("Fehler bei fork");
            exit(1);
        }
        else
        {
            /*
             * Vaterprozess
             * Keine spezielle Aktion erforderlich
             */
        }
    }

    return 0;
}


//c Interpretation

/*Die Ausgabe zeigt, dass keine Synchronisation zwischen den Prozessen 
stattfindet. Mehrere Prozesse können gleichzeitig den kritischen Bereich 
betreten. Deshalb ist der kritische Bereich in diesem Programm nicht 
wirklich geschützt, sondern nur durch Ausgaben simuliert. Die Reihenfolge 
der Ausgaben ist nicht deterministisch und kann bei jedem Programmlauf 
unterschiedlich sein, da das Betriebssystem entscheidet, welcher Prozess 
wann weiterläuft.*/