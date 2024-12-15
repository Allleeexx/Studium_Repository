// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The board model
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include "worm.h"
#include "board_model.h"
#include "messages.h"

// Place an item onto the curses display.
void placeItem(struct board* aboard, int y, int x, enum BoardCodes board_code, chtype symbol, enum ColorPairs color_pair) {

    //  Store item on the display
    move(y, x);
    attron(COLOR_PAIR(color_pair));
    addch(symbol);
    attroff(COLOR_PAIR(color_pair));
    aboard->cells[y][x] = board_code;
}

// Getters

// Get the last usable row on the display
int getLastRowOnBoard(struct board* aboard) {
    return aboard->last_row;     //@003 Da erste Zeile Index = 0, ist letzte Zeile Anzahl-1
}

// Get the last usable column on the display
int getLastColOnBoard(struct board* aboard) {
    return aboard->last_col;        //@004 Da erste Spalte Index = 0, ist letzte Spalte Anzahl-1
}

int getNumberOfFoodItems(struct board* aboard) {
    return aboard->food_items;
}

enum BoardCodes getContentAt(struct board* aboard, struct pos position) {
     return aboard->cells[position.y][position.x];
}

//Setters

void setNumberOfFoodItems(struct board* aboard, int n) {
     aboard->food_items = n;    // Zuweisung von n
}

void decrementNumberOfFoodItems(struct board* aboard) {
     aboard->food_items--;
}


int generateRandomNumber(int N) {
    static int seedSet = 0;
        if (!seedSet) {
        srand(time(NULL));
               seedSet = 1; }
    return rand() % (N + 1);
}

enum ResCodes initializeBoard(struct board* aboard) {
     int y;
     // Maximal index of a row, reserve space for message area
     aboard->last_row = LINES - ROWS_RESERVED - 1;
     // Maximal index of a column
     aboard->last_col = COLS - 1;

     // Check dimensions of the board
     if ( aboard->last_col < MIN_NUMBER_OF_COLS - 1 ||
     aboard->last_row < MIN_NUMBER_OF_ROWS - 1) {
     char buf[100];
     sprintf(buf, "Das Fenster ist zu klein: wir brauchen %dx%d",
     MIN_NUMBER_OF_COLS , MIN_NUMBER_OF_ROWS + ROWS_RESERVED );
     showDialog(buf, "Bitte eine Taste druecken");
     return RES_FAILED;
}
    // Allocate memory for 2-dimensional array of cells
    // Alloc array of rows
    aboard->cells = malloc(LINES * sizeof(enum BoardCodes));
    if (aboard->cells == NULL) {
        showDialog("Abbruch: Zu wenig Speicher","Bitte eine Taste druecken");
        exit(RES_FAILED); // No memory -> direct exit
}
    for (y = 0; y < LINES; y++) {
        // Allocate array of columns for each y
        aboard->cells[y] = malloc(COLS * sizeof(enum BoardCodes));
        if (aboard->cells[y] == NULL) {
           showDialog("Abbruch: Zu wenig Speicher","Bitte eine Taste druecken");
           exit(RES_FAILED); // No memory -> direct exit
        }
    }
    return RES_OK;
}


void cleanupBoard(struct board* aboard) {
    // Prüfe ob cells initialisiert wurde
    if (aboard->cells != NULL) {
        // Iteriere durch alle zeilen und gib sie frei
        for (int y = 0; y <= aboard->last_row; y++) {
            if (aboard->cells[y] != NULL) {
                free(aboard->cells[y]);
                aboard->cells[y] = NULL;
            }
        }
        // Gib das array der zeilen frei
        free(aboard->cells);
        aboard->cells = NULL;
    }
}


enum ResCodes initializeLevel(struct board* aboard)  {
      // define local variables for loops etc
    // Fill board and screen buffer with empty cells.
    for (int y = 0; y<=aboard->last_row ; y++) {
        for (int x = 0; x<=aboard->last_col ; x++) {
             placeItem(aboard, y, x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
     }
}
// Draw a line in order to separate the message area
// Note: we cannot use function placeItem() since the message are is outside the board!
int y = aboard->last_row + 1;
for (int x=0; x<=aboard->last_col ; x++) {
    move(y, x);
    attron(COLOR_PAIR(COLP_BARRIER));
    addch(SYMBOL_BARRIER);
    attroff(COLOR_PAIR(COLP_BARRIER));
}

// Barriers: use a loop
for (y = 15; y <= 20; y++ ) {
    int x = aboard->last_col * 0.3 ;
    placeItem(aboard, y, x, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
}
for (y = 10; y <= 20; y++ ) {
    int x = aboard->last_col - aboard->last_col * 0.3 ;
    placeItem(aboard, y, x, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
}
// Food
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);

placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);

placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
placeItem(aboard, generateRandomNumber(LINES -5), generateRandomNumber(COLS -2), BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);

// Initialize number of food items
// Attention: must match number of items placed on the baord above
aboard->food_items = 15;
return RES_OK;
}
