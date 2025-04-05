#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define ROWS 24
#define COLS 36

char spielfeld[ROWS][COLS];
int score = 0;
int playerRow, playerCol;
int fruitRow, fruitCol;
int tailX[100], tailY[100];
int tailLength = 0;
char currentDirection = 'n'; // Startet ohne Bewegung, 'n' für "none"
int maxScore = 0;

void initializeSpielfeld() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == 0 || j == 0 || i == ROWS - 1 || j == COLS - 1) {
                spielfeld[i][j] = '*';
            } else {
                spielfeld[i][j] = ' ';
            }
        }
    }
    generateRandomFruit();
    generatePlayer();
}

void showSpielfeld() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int isTail = 0;
            for (int k = 0; k < tailLength; k++) {
                if (tailX[k] == j && tailY[k] == i) {
                    isTail = 1;
                    break;
                }
            }
            if (i == playerRow && j == playerCol) {
                printf("A");
            } else if (isTail) {
                printf("-");
            } else {
                printf("%c", spielfeld[i][j]);
            }
        }
        printf("\n");
    }
}

void generateRandomFruit() {
    fruitRow = rand() % (ROWS - 2) + 1;
    fruitCol = rand() % (COLS - 2) + 1;
    spielfeld[fruitRow][fruitCol] = 'X';
}

void generatePlayer() {
    playerRow = ROWS / 2;
    playerCol = COLS / 2;
    spielfeld[playerRow][playerCol] = 'A';
}

void gameOver(){
    printf("GAME OVER! \n");
    printf("Dein Score war %d \n\n", score);
    if(score > maxScore){
        maxScore = score;
    }
    printf("Rekord: %d\n", maxScore);

    printf("Möchtest du noch eine weiter Runde spielen?\n");
    printf("Druecke 1 fuer Ja\n");
    printf("Druecke 2 fuer Nein \n");

    int choice;
    scanf("%d", &choice);
    switch(choice){
    case 1:
        score = 0;
        tailLength = 0;
        currentDirection = 'n';
        main();
        break;
    case 2:
        system("pause");
        exit(0);
    }
}

void movePlayer() {
    int newHeadRow = playerRow;
    int newHeadCol = playerCol;

    switch (currentDirection) {
        case 'w':
            newHeadRow--;
            break;
        case 'a':
            newHeadCol--;
            break;
        case 's':
            newHeadRow++;
            break;
        case 'd':
            newHeadCol++;
            break;
    }

    // Check collision with walls
    if (newHeadRow == 0 || newHeadRow == ROWS - 1 || newHeadCol == 0 || newHeadCol == COLS - 1) {
        gameOver();
    }

    // Check collision with tail
    for (int k = 0; k < tailLength; k++) {
        if (newHeadRow == tailY[k] && newHeadCol == tailX[k]) {
           gameOver();
        }
    }

    // Update tail positions
    for (int i = tailLength - 1; i > 0; i--) {
        tailX[i] = tailX[i - 1];
        tailY[i] = tailY[i - 1];
    }
    if (tailLength > 0) {
        tailX[0] = playerCol;
        tailY[0] = playerRow;
    }

    // Clear the old head position on the field
    spielfeld[playerRow][playerCol] = ' ';

    // Check collision with fruit
    if (newHeadRow == fruitRow && newHeadCol == fruitCol) {
        score += 10;
        tailLength++;
        if (tailLength > 1) {
            tailX[tailLength - 1] = playerCol;
            tailY[tailLength - 1] = playerRow;
        }
        generateRandomFruit();
    }

    // Move player
    playerRow = newHeadRow;
    playerCol = newHeadCol;
    spielfeld[playerRow][playerCol] = 'A'; // Set the new head position
}


void displayScore() {
    printf("Score: %d\n", score);
    printf("Rekord: %d\n", maxScore);
}

int main() {
    srand(time(NULL)); // Initialisiere Zufallsgenerator einmal
    initializeSpielfeld();
    while (1) {
        if (kbhit()) {
            char input = getch();
            // Akzeptiere die neue Richtung nur, wenn sie nicht der entgegengesetzten Richtung entspricht
            switch (input) {
                case 'w': case 'W':
                    if (currentDirection != 's') // Nicht nach unten, wenn man nach oben geht
                        currentDirection = 'w';
                    break;
                case 's': case 'S':
                    if (currentDirection != 'w') // Nicht nach oben, wenn man nach unten geht
                        currentDirection = 's';
                    break;
                case 'a': case 'A':
                    if (currentDirection != 'd') // Nicht nach rechts, wenn man nach links geht
                        currentDirection = 'a';
                    break;
                case 'd': case 'D':
                    if (currentDirection != 'a') // Nicht nach links, wenn man nach rechts geht
                        currentDirection = 'd';
                    break;
                default:
                    // Ignoriere andere Tasten
                    break;
            }
        }
        system("cls");
        movePlayer();
        showSpielfeld();
        displayScore();
    }
    return 0;
}


