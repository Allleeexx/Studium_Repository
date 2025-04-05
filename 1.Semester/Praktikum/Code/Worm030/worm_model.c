// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The worm model

#include <curses.h>
#include "board_model.h"
#include "worm.h"
#include "worm_model.h"


// Data defining the worm
int theworm_wormpos_y[WORM_LENGTH];  // Array das von y Koordinaten (groeße 20)
int theworm_wormpos_x[WORM_LENGTH];  // Array das von x Koordinaten (groeße 20)
int theworm_maxindex; //letzte möglich verwendbare spalten/zeilen in en Arrays
int theworm_headindex; //Index speichern, in der die Koordinaten des Kopfes gespeichert sind

// The current heading of the worm
// These are offsets from the set {-1,0,+1}
int theworm_dx;
int theworm_dy;
enum ColorPairs theworm_wcolor;


//Code für folgende Funktionen

// Initialize the worm
extern enum ResCodes initializeWorm(int len_max /*maximale lange des Wurms*/, int headpos_y, int headpos_x, enum WormHeading dir, enum ColorPairs color) {
    // Local variables for loops , intialisiere wurm passiert einmal

    // Initialize last usable index to len_max -1
    // theworm_maxindex
    theworm_maxindex = len_max-1; //Weil beim Index zahlen auch bei 0 beginnt

    // Initalize headindex
    // theworm_headindex
    theworm_headindex = 0;

    // Mark all elements as unused in the arrays of positions
    // theworm_wormpos_y[] and thewormpos_x[]
    // An unused position in the array is marked
    // with code UNUSED_POS_ELEM
    for (int i=0; i<= theworm_maxindex; i++) {
      theworm_wormpos_y[i]= UNUSED_POS_ELEM; // befülle das Array mit Unused, da wir noch nix befüllt haben
      theworm_wormpos_x[i] = UNUSED_POS_ELEM;
    }

    // Initialize position of worms head
    theworm_wormpos_y[theworm_headindex] = headpos_y; // das array an der stelle des Headindexes soll befüllt werden mit der y koordinate
    theworm_wormpos_x[theworm_headindex] = headpos_x;

    // Initialize the heading of the worm
    setWormHeading(dir); /* @009*/

    // Initialze color of the worm
    theworm_wcolor = color;
    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
extern void showWorm() {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
    placeItem(
            theworm_wormpos_y[theworm_headindex] ,
            theworm_wormpos_x[theworm_headindex] /* @007*/,
            SYMBOL_WORM_INNER_ELEMENT,theworm_wcolor);
}

// Clean worm Tail
extern void cleanWormTail(){
  int tailindex;
  // Compute tailindex
  tailindex = (theworm_headindex + 1) % (theworm_maxindex +1);

  //Check the array of the worm elements.
  //Is the array element at tailindex already in use?
  //Checking either array the_wormpos_y
  //or theworm_wormpos_x is enough.
  if (theworm_wormpos_y[tailindex] != UNUSED_POS_ELEM) {
    //YES: place a SYMBOL_FREE_CELL at the tails positions
    placeItem(theworm_wormpos_y[tailindex], theworm_wormpos_x[tailindex],
    SYMBOL_FREE_CELL, COLP_FREE_CELL); //Das ist nur visuell, die Daten vom Tail sind immernoch in der Tabelle
  }
}


extern void moveWorm(enum GameStates* agame_state) {
    // Compute and store new head position according to current heading.
    int headpos_x = theworm_wormpos_x[theworm_headindex] + theworm_dx;
    int headpos_y = theworm_wormpos_y[theworm_headindex] + theworm_dy;

    // Check if we would leave the display if we move the worm's head according
    // to worm's last direction.
    // We are not allowed to leave the display's window.

    if (headpos_x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos_x > getLastCol() ) {
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else if (headpos_y < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS; /* @011*/
	} else if (headpos_y > getLastRow() ) {
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else {
        // We will stay within bounds.
        // Check if the worm's head will collide with itself at the new position
        if (isInUseByWorm(headpos_y, headpos_x)) {
          // Thats bad: stop the game
          *agame_state = WORM_CROSSING;
        }
    }

    // Check the status of *agame_state
    // Go on if nothing bad happened
    if ( *agame_state == WORM_GAME_ONGOING) {
      //So all is well: we did not hit anything bad and did not leave the window. --> Update worm structure
      //Increment the worm_headindex
      //Go round if end of the worm is reached (ring buffer)
      theworm_headindex = (theworm_headindex +1) % (theworm_maxindex +1);

      // Store new corrdinates of head element in worm structure
      theworm_wormpos_x[theworm_headindex] = headpos_x;
      theworm_wormpos_y[theworm_headindex] = headpos_y;
    }
}


extern bool isInUseByWorm(int new_headpos_y, int new_headpos_x) {
    int i;
    bool collision = false;
    i = theworm_headindex;
    do{                              //Do while = TUE ES EINMAL bevor du die Bedingung durchgehst, bei while wird die bedingung direkt beim ersten durchgehen gecheckt
      // Compare the position of the currrent worm element with the new_headpos
      if(theworm_wormpos_y[i] == new_headpos_y && theworm_wormpos_x[i] == new_headpos_x) {
        collision = true;
      }
      i= (i+1) % (theworm_maxindex +1);
    }
    while (i != theworm_headindex ); // die x koordinate dürfen nicht gleich sein, aber die y schon (gleiche hohe)
    // return what we found out.
    return collision;
}

extern void setWormHeading(enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :    // User wants up
            theworm_dx=0;
            theworm_dy=-1;
            break;
        case WORM_DOWN :  // User wants down
            theworm_dx=0; // @005;
            theworm_dy=+1; // @005;
            break;
        case WORM_LEFT :  //@005, User wants left
            theworm_dx=-1; // @005
            theworm_dy=0; // @005
            break;
        case WORM_RIGHT : // @005, User wants right
           theworm_dx=+1; // @005
           theworm_dy=0; // @005
            break;

        case WORM_LEFT_UP :
            theworm_dx=-1;
            theworm_dy=-1;
            break;
        case WORM_RIGHT_UP :
            theworm_dx=+1;
            theworm_dy=-1;
            break;
        case WORM_LEFT_DOWN :
            theworm_dx=-1;
            theworm_dy=+1;
            break;
        case WORM_RIGHT_DOWN :
            theworm_dx=+1;
            theworm_dy=+1;
            break;
    }
}
