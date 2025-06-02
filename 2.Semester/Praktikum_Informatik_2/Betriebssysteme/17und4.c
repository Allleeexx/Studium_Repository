#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_PLAYERS 4
#define MAX_SCORE 21
#define WIN_THRESHOLD 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t round_start = PTHREAD_COND_INITIALIZER;
pthread_cond_t round_end = PTHREAD_COND_INITIALIZER;

int player_scores[MAX_PLAYERS];
int player_wins[MAX_PLAYERS];
int bank_score = 0;
int num_players;
int round_ready = 0;
int round_done = 0;
int game_over = 0;

int draw_card() {
    return rand() % 10 + 2; // Werte zwischen 2 und 11
}

void* player_thread(void* arg) {
    int id = *(int*)arg;

    while (!game_over) {
        pthread_mutex_lock(&mutex);

        while (!round_ready && !game_over)
            pthread_cond_wait(&round_start, &mutex);
        if (game_over) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Spieler zieht Karten
        player_scores[id] = 0;
        while (player_scores[id] < 17)
            player_scores[id] += draw_card();

        round_done++;
        if (round_done == num_players)
            pthread_cond_signal(&round_end);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* bank_thread(void* arg) {
    while (!game_over) {
        sleep(1); // kurze Pause zwischen Runden
        pthread_mutex_lock(&mutex);

        // Neue Runde vorbereiten
        round_ready = 1;
        round_done = 0;
        pthread_cond_broadcast(&round_start);

        // Bank zieht Karten
        bank_score = 0;
        while (bank_score < 17)
            bank_score += draw_card();

        // Warten, bis alle Spieler fertig sind
        while (round_done < num_players)
            pthread_cond_wait(&round_end, &mutex);

        printf("\n--- Neue Runde ---\n");
        printf("Bank hat: %d\n", bank_score);

        // Gewinner ermitteln
        for (int i = 0; i < num_players; i++) {
            printf("Spieler %d hat: %d\n", i + 1, player_scores[i]);

            if (player_scores[i] > MAX_SCORE) continue; // Spieler überkauft
            if (bank_score > MAX_SCORE || player_scores[i] > bank_score)
                player_wins[i]++;
        }

        // Spielstand anzeigen
        for (int i = 0; i < num_players; i++)
            printf("Spieler %d hat %d Siege\n", i + 1, player_wins[i]);

        // Hat jemand gewonnen?
        for (int i = 0; i < num_players; i++) {
            if (player_wins[i] >= WIN_THRESHOLD) {
                printf("\n🎉 Spieler %d hat das Spiel gewonnen! 🎉\n", i + 1);
                game_over = 1;
                pthread_cond_broadcast(&round_start);
                break;
            }
        }

        round_ready = 0;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    printf("Anzahl der Spieler (2-%d): ", MAX_PLAYERS);
    scanf("%d", &num_players);
    if (num_players < 2 || num_players > MAX_PLAYERS) {
        printf("Ungültige Spieleranzahl!\n");
        return 1;
    }

    pthread_t players[MAX_PLAYERS];
    pthread_t bank;
    int ids[MAX_PLAYERS];

    pthread_create(&bank, NULL, bank_thread, NULL);
    for (int i = 0; i < num_players; i++) {
        ids[i] = i;
        pthread_create(&players[i], NULL, player_thread, &ids[i]);
    }

    pthread_join(bank, NULL);
    for (int i = 0; i < num_players; i++)
        pthread_join(players[i], NULL);

    printf("Spiel beendet.\n");
    return 0;
}
