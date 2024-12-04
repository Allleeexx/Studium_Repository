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
#include "worm.h"
#include "board_model.h"


// *************************************************
// Placing and removing items from the game board
// Check boundaries of game board
// *************************************************

// Place an item onto the curses display.
void placeItem(int y, int x, chtype symbol, enum ColorPairs color_pair) {

    //  Store item on the display (symbol code)
    move(y, x);                         // Move cursor to (y,x)
    attron(COLOR_PAIR(color_pair));     // Start writing in selected color
    addch(symbol /*@006*/);                      // Store symbol on the virtual display
    attroff(COLOR_PAIR(color_pair));    // Stop writing in selected color
}

// Getters

// Get the last usable row on the display
int getLastRowOnBoard(struct board* aboard){
    return aboard -> last_row;
}

// Get the last usable column on the display
int getLastColOnBoard(struct board* aboard){
    return aboard -> last_col;
}

int getNumberOfFoodItems(struct board* aboard){
    return aboard -> food_items;
}

enum BoardCodes getContentAt(struct board* aboard, struct pos position){
    return aboard -> cells[position.y][position.x];
}

void setNumberOfFoodItems (struct board* aboard, int n){
    aboard->food_items = n;
}

void decrementNumberOfFoodItems(struct board* aboard){
    aboard->food_items -= 1;
}
