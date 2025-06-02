#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_PLAYERS 4
#define MAX_SCORE 21
#define WIN_THRESHOLD 3

struct SharedData {
    int player_scores[MAX_PLAYERS];
    int player_wins[MAX_PLAYERS];
    int bank_score;
    int ready_flags[MAX_PLAYERS];
    int game_over;
};

int draw_card() {
    return rand() % 10 + 2;
}

void player_process(int player_id, struct SharedData *data) {
    srand(getpid());

    while (1) {
        while (data->ready_flags[player_id] != 0 && !data->game_over)
            usleep(1000);

        if (data->game_over)
            break;

        // Karten ziehen
        data->player_scores[player_id] = 0;
        while (data->player_scores[player_id] < 17)
            data->player_scores[player_id] += draw_card();

        // Spieler ist bereit
        data->ready_flags[player_id] = 1;
    }
    exit(0);
}

int main() {
    int shm_id = shmget(IPC_PRIVATE, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    struct SharedData *data = (struct SharedData *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialisierung
    for (int i = 0; i < MAX_PLAYERS; i++) {
        data->player_scores[i] = 0;
        data->player_wins[i] = 0;
        data->ready_flags[i] = 0;
    }
    data->game_over = 0;

    int num_players;
    printf("Anzahl der Spieler (2–%d): ", MAX_PLAYERS);
    scanf("%d", &num_players);
    if (num_players < 2 || num_players > MAX_PLAYERS) {
        printf("Ungültige Spieleranzahl!\n");
        exit(1);
    }

    // Prozesse starten
    for (int i = 0; i < num_players; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            player_process(i, data);
            exit(0);
        }
    }

    srand(time(NULL));

    // Bank (Hauptprozess)
    while (!data->game_over) {
        sleep(1);  // kurze Pause vor Runde
        printf("\n--- Neue Runde ---\n");

        // Spieler freigeben
        for (int i = 0; i < num_players; i++)
            data->ready_flags[i] = 0;

        // Bank zieht Karten
        data->bank_score = 0;
        while (data->bank_score < 17)
            data->bank_score += draw_card();
        printf("Bank hat: %d\n", data->bank_score);

        // Warten auf alle Spieler
        int all_ready = 0;
        while (!all_ready) {
            all_ready = 1;
            for (int i = 0; i < num_players; i++) {
                if (data->ready_flags[i] == 0)
                    all_ready = 0;
            }
            usleep(1000);
        }

        // Auswertung
        for (int i = 0; i < num_players; i++) {
            int score = data->player_scores[i];
            printf("Spieler %d hat: %d\n", i + 1, score);

            if (score > MAX_SCORE) continue;
            if (data->bank_score > MAX_SCORE || score > data->bank_score)
                data->player_wins[i]++;
        }

        // Spielstand
        for (int i = 0; i < num_players; i++) {
            printf("Spieler %d hat %d Siege\n", i + 1, data->player_wins[i]);
        }

        // Gewinner prüfen (nur erster zählt!)
        for (int i = 0; i < num_players; i++) {
            if (data->player_wins[i] >= WIN_THRESHOLD) {
                printf("\n🎉 Spieler %d hat das Spiel gewonnen! 🎉\n", i + 1);
                data->game_over = 1;
                break; // sofort beenden, nur einer gewinnt
            }
        }
    }

    // Kindprozesse beenden
    for (int i = 0; i < num_players; i++)
        wait(NULL);

    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("Spiel beendet.\n");
    return 0;
}
