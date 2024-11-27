// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//

#ifndef _WORM_H
#define _WORM_H

// Result codes of functions
enum ResCodes {         //Erstelle einen Neuen "Datentypen" , wobei RES_OK Automatisch mit 0 def wird und dann jedes weitere +1
  RES_OK,
  RES_FAILED,
};

// Dimensions and bounds
#define NAP_TIME    100   // Time in milliseconds to sleep between updates of display
#define MIN_NUMBER_OF_ROWS  3   // The guaranteed number of rows available for the board
#define MIN_NUMBER_OF_COLS 10   // The guaranteed number of columns available for the board
#define WORM_LENGTH 20 // maximale Wurmlange

// ### Codes for the array of possitions ###
// unused element in the worm arrays of positions
#define UNUSED_POS_ELEM -1


// Numbers for color pairs used by curses macro COLOR_PAIR
enum ColorPairs {
  COLP_USER_WORM = 1,
  COLP_FREE_CELL,
};

// Symbols to display
#define SYMBOL_FREE_CELL ' '
#define SYMBOL_WORM_INNER_ELEMENT '0'

// Game state codes
enum GameStates {
  WORM_GAME_ONGOING,
  WORM_OUT_OF_BOUNDS, // Left screen
  WORM_CROSSING,
  WORM_GAME_QUIT,     // user likes to quit
};

#endif  // #define _WORM_H
