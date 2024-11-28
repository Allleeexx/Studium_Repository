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


// Initialize the worm
extern enum ResCodes initializeWorm(struct worm* aworm,int len_max ,struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
    // Local variables for loops , intialisiere wurm passiert einmal

    // Initialize last usable index to len_max -1
    // theworm_maxindex
    aworm -> maxindex = len_max-1; //Weil beim Index zahlen auch bei 0 beginnt

    // Initalize headindex
    aworm -> headindex = 0;

    // Mark all elements as unused in the arrays of positions
    // theworm_wormpos_y[] and thewormpos_x[]
    // An unused position in the array is marked
    // with code UNUSED_POS_ELEM
    for (int i=0; i<= aworm -> maxindex; i++) {
      aworm -> wormpos[i].y = UNUSED_POS_ELEM; // befülle das Array mit Unused, da wir noch nix befüllt haben
      aworm -> wormpos[i].x = UNUSED_POS_ELEM;
    }

    // Initialize position of worms head
    aworm -> wormpos[aworm -> headindex] = headpos; 

    // Initialize the heading of the worm
    setWormHeading(aworm, dir); /* @009*/

    // Initialze color of the worm
    aworm -> wcolor = color;
    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
extern void showWorm(struct worm* aworm) {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
    placeItem(
            aworm -> wormpos[aworm -> headindex].y ,
            aworm -> wormpos[aworm -> headindex].x /* @007*/,
            SYMBOL_WORM_INNER_ELEMENT,aworm -> wcolor);
}

// Clean worm Tail
extern void cleanWormTail(struct worm* aworm){
  int tailindex;
  // Compute tailindex
  tailindex = (aworm ->headindex + 1) % (aworm ->maxindex +1);

  //Check the array of the worm elements.
  //Is the array element at tailindex already in use?
  //Checking either array the_wormpos_y
  //or aworm ->wormpos_x is enough.
  if (aworm ->wormpos[tailindex].y != UNUSED_POS_ELEM) {
    //YES: place a SYMBOL_FREE_CELL at the tails positions
    placeItem(aworm ->wormpos[tailindex].y, aworm ->wormpos[tailindex].x,
    SYMBOL_FREE_CELL, COLP_FREE_CELL); //Das ist nur visuell, die Daten vom Tail sind immernoch in der Tabelle
  }
}


extern void moveWorm(struct worm* aworm, enum GameStates* agame_state) {
    // Compute and store new head position according to current heading.
    struct pos headpos;

    headpos.x = aworm -> wormpos[aworm->headindex].x + aworm->dx;
    headpos.y = aworm -> wormpos[aworm->headindex].y + aworm->dy;

    // Check if we would leave the display if we move the worm's head according
    // to worm's last direction.
    // We are not allowed to leave the display's window.

    if (headpos.x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.x > getLastCol() ) {
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else if (headpos.y < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS; /* @011*/
	} else if (headpos.y > getLastRow() ) {
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else {
        // We will stay within bounds_
        // Check if the worm's head will collide with itself at the new position
        if (isInUseByWorm(aworm, &headpos)) {
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
      aworm -> headindex = (aworm -> headindex +1) % (aworm -> maxindex +1);

      // Store new corrdinates of head element in worm structure
      aworm -> wormpos[aworm -> headindex].x = headpos.x;
      aworm -> wormpos[aworm -> headindex].y = headpos.y;
    }
}


extern bool isInUseByWorm(struct worm* aworm, struct pos* apos) {
    int i;
    bool collision = false;
    i = aworm -> headindex;
    do{                              //Do while = TUE ES EINMAL bevor du die Bedingung durchgehst, bei while wird die bedingung direkt beim ersten durchgehen gecheckt
      // Compare the position of the currrent worm element with the new_headpos
      if(aworm -> wormpos[i].y == apos -> y && aworm -> wormpos[i].x == apos -> x) {
        collision = true;
      }
      i= (i+1) % (aworm -> maxindex +1);
    }
    while (i != aworm -> headindex ); // die x koordinate dürfen nicht gleich sein, aber die y schon (gleiche hohe)
    // return what we found out.
    return collision;
}

extern void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :    // User wants up
            aworm -> dx=0;
            aworm -> dy=-1;
            break;
        case WORM_DOWN :  // User wants down
            aworm -> dx=0; // @005;
            aworm -> dy=+1; // @005;
            break;
        case WORM_LEFT :  //@005, User wants left
            aworm -> dx=-1; // @005
            aworm -> dy=0; // @005
            break;
        case WORM_RIGHT : // @005, User wants right
           aworm -> dx=+1; // @005
           aworm -> dy=0; // @005
            break;

        case WORM_LEFT_UP :
            aworm -> dx=-1;
            aworm -> dy=-1;
            break;
        case WORM_RIGHT_UP :
            aworm -> dx=+1;
            aworm -> dy=-1;
            break;
        case WORM_LEFT_DOWN :
            aworm -> dx=-1;
            aworm -> dy=+1;
            break;
        case WORM_RIGHT_DOWN :
            aworm -> dx=+1;
            aworm -> dy=+1;
            break;
    }
}
