#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

/*
 * Wird für semctl() mit SETVAL benötigt.
 * Auf manchen Systemen ist union semun nicht vordefiniert.
 */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/*
 * Globale Semaphor-ID, damit init_sem(), P() und V()
 * darauf zugreifen können.
 */
int semid;

/*
 * init_sem():
 * Initialisiert das Semaphor mit dem Wert 1.
 * 1 bedeutet: Betriebsmittel frei.
 */
void init_sem(void)
{
    union semun arg;

    arg.val = 1;

    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("Fehler bei semctl SETVAL");
        exit(1);
    }
}

/*
 * P-Operation:
 * Versucht, das Semaphor zu dekrementieren.
 * Wenn der Wert 0 ist, wartet der Prozess.
 */
void P(int sem_num)
{
    struct sembuf operation[1];

    operation[0].sem_num = sem_num;
    operation[0].sem_op = -1;
    operation[0].sem_flg = 0;

    if (semop(semid, operation, 1) == -1)
    {
        perror("Fehler bei semop P");
        exit(1);
    }
}

/*
 * V-Operation:
 * Inkrementiert das Semaphor wieder.
 * Dadurch wird der kritische Bereich freigegeben.
 */
void V(int sem_num)
{
    struct sembuf operation[1];

    operation[0].sem_num = sem_num;
    operation[0].sem_op = 1;
    operation[0].sem_flg = 0;

    if (semop(semid, operation, 1) == -1)
    {
        perror("Fehler bei semop V");
        exit(1);
    }
}

int main(void)
{
    pid_t pid;
    key_t key;
    union semun arg;

    /*
     * b) Eindeutigen Schlüssel mit ftok() erzeugen.
     * "." muss existieren, deshalb wird hier das aktuelle Verzeichnis verwendet.
     */
    key = ftok(".", 1);

    if (key == -1)
    {
        perror("Fehler bei ftok");
        exit(1);
    }

    /*
     * Zugriff auf eine Gruppe bestehend aus einem Semaphor.
     * IPC_CREAT | 0666:
     * Semaphor wird erzeugt, falls es noch nicht existiert.
     */
    semid = semget(key, 1, IPC_CREAT | 0666);

    if (semid == -1)
    {
        perror("Fehler bei semget");
        exit(1);
    }

    /*
     * c) Semaphor mit 1 initialisieren.
     */
    init_sem();

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
                /*
                 * Kritischen Bereich betreten.
                 * Vorher wird P() ausgeführt.
                 * Falls bereits ein anderer Prozess im kritischen Bereich ist,
                 * muss dieser Prozess hier warten.
                 */
                P(0);

                printf("Prozess %d betritt kritischen Bereich\n", i);
                sleep(1);
                printf("Prozess %d verlaesst kritischen Bereich\n", i);

                /*
                 * Kritischen Bereich verlassen.
                 * Danach wird V() ausgeführt.
                 */
                V(0);

                /*
                 * Unkritischer Bereich bleibt unsynchronisiert.
                 */
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

    /*
     * Vaterprozess wartet auf alle Sohnprozesse.
     * Das ist wichtig, damit das Semaphor erst danach gelöscht wird.
     */
    for (int i = 1; i <= 3; i++)
    {
        if (wait(NULL) == -1)
        {
            perror("Fehler bei wait");
            exit(1);
        }
    }

    /*
     * Semaphor wieder löschen.
     */
    if (semctl(semid, 0, IPC_RMID, arg) == -1)
    {
        perror("Fehler bei semctl IPC_RMID");
        exit(1);
    }

    return 0;
}