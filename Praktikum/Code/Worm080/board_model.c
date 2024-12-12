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

#include <curses.h>
#include "board_model.h"
#include "worm.h"
#include "messages.h"

// Place an item onto the curses display.
void placeItem(struct board* aboard, int y, int x, enum BoardCodes board_code, chtype symbol, enum ColorPairs color_pair) {

    //  Store item on the display (symbol code)
    move(y, x);                         // Move cursor to (y,x)
    attron(COLOR_PAIR(color_pair));     // Start writing in selected color
    addch(symbol /*@006*/);                      // Store symbol on the virtual display
    attroff(COLOR_PAIR(color_pair));    // Stop writing in selected color
    aboard-> cells[y][x] = board_code;
}

enum ResCodes initializeBoard(struct board* aboard) {
    // Check dimension of the board
    if (COLS < MIN_NUMBER_OF_COLS || LINES < MIN_NUMBER_OF_ROWS + ROWS_RESERVED) {
       char buf[100];
       sprintf(buf, "Das Fenster ist zu klein: wir benötigen %dx%d", MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS + ROWS_RESERVED);
       showDialog(buf, "Bitte eine Taste druecken");
       return RES_FAILED;
    }

    //Maximal index of a row
    aboard-> last_row = MIN_NUMBER_OF_ROWS -1;

    //Maximal index of a column
    aboard-> last_col = MIN_NUMBER_OF_COLS -1;
    return RES_OK;
}

enum ResCodes initializeLevel (struct board* aboard) {
    int y,x; // define local variables for loops

    //Fill board and screen buffer with empty cells.
    for (y=0 ; y<= aboard->last_row ; y++) {
      for (x=0; x<= aboard->last_col ; x++) {
          placeItem(aboard, y, x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
      }
    }

    //Draw a line in order to seperate the message area
    //Note: we cannot use funtion placeItem since the message area is outside the board
    y = aboard-> last_row +1;
    for (x=0; x<= aboard-> last_col ; x++) {
      move(y,x);
      attron(COLOR_PAIR(COLP_BARRIER));
      addch(SYMBOL_BARRIER);
      attroff(COLOR_PAIR(COLP_BARRIER));
    } 

    // Draw a line to signale the rightmost column of the board
    for(y=0; y<= aboard-> last_row; y++) {
      placeItem(aboard, y , aboard-> last_col, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
    }

    // Barriers: use a loop, one on left side, other right side
    for(y=2; y < 15; y++) {
      placeItem(aboard, y , aboard-> last_col/3, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
    }

    for(y= 8; y<20; y++) {
      placeItem(aboard, y , (aboard-> last_col/3 * 2) +(aboard-> last_col/12) , BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
    }

    //Food
    placeItem(aboard, 3 , 3, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
    placeItem(aboard, 9 , 13, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
    placeItem(aboard, 23 , 20, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
    placeItem(aboard,  6, 62, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
    placeItem(aboard,  12, 39, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
    
    placeItem(aboard, 17, 25, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
    placeItem(aboard, 23 , 3, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
    placeItem(aboard, 10 ,66 , BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
    placeItem(aboard, 4 , 33, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
    placeItem(aboard, 3 , 50, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);

    placeItem(aboard, 18 , 2, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
    placeItem(aboard, 21 , 45, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
    placeItem(aboard, 24 , 58, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
    placeItem(aboard, 5 , 17, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
    placeItem(aboard, 15 , 37, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);

    //Initialize number of food items
    //Attentionm: must match number of items placed on the board above
    aboard-> food_items= 15;
    return RES_OK;
}

// Getters

// Get the last usable row on the display
int getLastRowOnBoard(struct board* aboard) {
    return aboard-> last_row;
}

// Get the last usable column on the display
int getLastColOnBoard(struct board* aboard) {
    return aboard-> last_col;
}

// Get number of food items (available)
int getNumberOfFoodItems(struct board* aboard) {
    return aboard-> food_items;
}

enum BoardCodes getContentAt(struct board* aboard, struct pos position) {
    return aboard-> cells[position.y][position.x];
}

// Setters

void setNumberOfFoodItems(struct board* aboard, int n) {
    aboard-> food_items = n;
}

void decrementNumberOfFoodItems(struct board* aboard) {
    aboard-> food_items = aboard-> food_items-1;
}


