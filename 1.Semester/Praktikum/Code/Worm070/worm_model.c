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
#include "worm.h"
#include "board_model.h"
#include "worm_model.h"

enum ColorPairs theworm_wcolor; 

// Initialize the worm
enum ResCodes initializeWorm(struct worm* aworm, int len_max, int len_cur, struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
    // Local variables for loops , intialisiere wurm passiert einmal
  
    // Initialize last usable index to len_max -1
    // theworm_maxindex
    aworm->maxindex = len_max-1; //Weil beim Index zahlen auch bei 0 beginnt

    // Current last usable index in array. May grow upto maxindex
    aworm->cur_lastindex = len_cur -1;

    // Initalize headindex
    // theworm_headindex
    aworm->headindex = 0;

    // Mark all elements as unused in the arrays of positions
    // theworm_wormpos_y[] and thewormpos_x[]
    // An unused position in the array is marked
    // with code UNUSED_POS_ELEM
    for (int i=0; i<= aworm->maxindex; i++) {
      aworm->wormpos[i].y = UNUSED_POS_ELEM; // befülle das Array mit Unused, da wir noch nix befüllt haben
      aworm->wormpos[i].x = UNUSED_POS_ELEM;
    }

    // Initialize position of worms head
    aworm->wormpos[aworm->headindex] = headpos; // das array an der stelle des Headindexes soll befüllt werden

    // Initialize the heading of the worm
    setWormHeading(aworm, dir); /* @009*/

    // Initialze color of the worm
    aworm->wcolor = color;
    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
void showWorm(struct board* aboard, struct worm* aworm) {
   
    int i;
    i = aworm->headindex;

    int tailindex;
    tailindex= (aworm->headindex + 1) % (aworm->cur_lastindex +1);
    
    do{
      if(aworm->wormpos[i].x != UNUSED_POS_ELEM) {
         if(i == aworm->headindex) {
           placeItem(aboard, aworm->wormpos[i].y, aworm->wormpos[i].x, BC_USED_BY_WORM, SYMBOL_WORM_HEAD_ELEMENT,aworm->wcolor);
         } else if (i == tailindex) {
           placeItem(aboard, aworm-> wormpos[i].y, aworm-> wormpos[i].x, BC_USED_BY_WORM, SYMBOL_WORM_TAIL_ELEMENT, aworm-> wcolor);
         } else if (i <= aworm-> cur_lastindex) {
           placeItem(aboard, aworm-> wormpos[i].y, aworm-> wormpos[i].x, BC_USED_BY_WORM, SYMBOL_WORM_INNER_ELEMENT, aworm-> wcolor);
         }
      } 
      i = ((i-1)+ (aworm->cur_lastindex+1)) % (aworm->cur_lastindex +1);
    }
    while (i != aworm->headindex);
}

void cleanWormTail(struct board* aboard, struct worm* aworm) {
  int tailindex;
  // Compute tailindex
  tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex +1);

  //Check the array of the worm elements.
  //Is the array element at tailindex already in use?
  //Checking either array the_wormpos_y
  //or theworm_wormpos_x is enough.
  if (aworm->wormpos[tailindex].y != UNUSED_POS_ELEM) {
    //YES: place a SYMBOL_FREE_CELL at the tails positions
    placeItem(aboard, aworm->wormpos[tailindex].y,aworm->wormpos[tailindex].x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL); //Das ist nur visuell, die Daten vom Tail sind immernoch in der Tabelle
  } 
}

void moveWorm(struct board* aboard, struct worm* aworm, enum GameStates* agame_state) {
    // Compute and store new head position according to current heading.
    struct pos headpos;

    headpos = aworm-> wormpos[aworm->headindex];
    
    headpos.x  += aworm->dx;
    headpos.y  += aworm->dy;

    // Check if we would leave the display if we move the worm's head according
    // to worm's last direction.
    // We are not allowed to leave the display's window.
    if (headpos.x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.x > getLastColOnBoard(aboard) ) { 
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else if (headpos.y < 0) {  
        *agame_state = WORM_OUT_OF_BOUNDS; /* @011*/
	} else if (headpos.y > getLastRowOnBoard(aboard) ) {
        *agame_state = WORM_OUT_OF_BOUNDS; /*@011*/
    } else {
        // We will stay within bounds.

        // Check if the worm's head hits any items at the new position on the board
        // Hitting food is good, hitting barrier or worm elements is bad
        switch (getContentAt(aboard, headpos)) {
            case BC_FOOD_1:
                *agame_state = WORM_GAME_ONGOING;
                //Grow worm according to food item
                growWorm(aworm, BONUS_1);
                decrementNumberOfFoodItems(aboard);
                break;

            case BC_FOOD_2:
                *agame_state = WORM_GAME_ONGOING;
                //Grow worm according to food item
                growWorm(aworm, BONUS_2);
                decrementNumberOfFoodItems(aboard);
                break;

            case BC_FOOD_3:
                *agame_state = WORM_GAME_ONGOING;
                //Grow worm according to food item
                growWorm(aworm, BONUS_3);
                decrementNumberOfFoodItems(aboard);
                break;

            case BC_BARRIER:
                *agame_state = WORM_CRASH;
                break;

            case BC_USED_BY_WORM:
                *agame_state = WORM_CROSSING;
                break;

            default:
                {;}
        }
    }

    
    // Check the status of *agame_state
    // Go on if nothing bad happened
    if ( *agame_state == WORM_GAME_ONGOING) {
      //So all is well: we did not hit anything bad and did not leave the window. --> Update worm structure
      //Increment the worm_headindex
      //Go round if end of the worm is reached (ring buffer)
       aworm->headindex++;
       if (aworm->headindex > aworm->cur_lastindex) {
         aworm-> headindex = 0;
       }

      // Store new corrdinates of head element in worm structure
      aworm-> wormpos[aworm-> headindex] = headpos; 
    }
}

void growWorm(struct worm* aworm, enum Boni growth) {
     //play it safe and inhibit surpassing the bound
     if(aworm->cur_lastindex + growth <= aworm-> maxindex) {
       aworm->cur_lastindex += growth;
     } else {
       aworm->cur_lastindex = aworm-> maxindex;
     }
}

// Setters
void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :    // User wants up
            aworm->dx=0;
            aworm->dy=-1;
            break;
        case WORM_DOWN :  // User wants down
            aworm->dx=0; // @005;
            aworm->dy=+1; // @005;
            break;
        case WORM_LEFT :  //@005, User wants left
            aworm->dx=-1; // @005
            aworm->dy=0; // @005
            break;
        case WORM_RIGHT : // @005, User wants right
            aworm->dx=+1; // @005
            aworm->dy=0; // @005
            break;

//        case WORM_LEFT_UP :
//            aworm->dx=-1;
//            aworm->dy=-1;
//            break;
//        case WORM_RIGHT_UP :
//            aworm->dx=+1;
//            aworm->dy=-1;
//            break;
//        case WORM_LEFT_DOWN :
//            aworm->dx=-1;
//            aworm->dy=+1;
//            break;
//        case WORM_RIGHT_DOWN :
//            aworm->dx=+1;
//            aworm->dy=+1;
//            break;
    }
} 
    
// Getters
struct pos getWormHeadPos(struct worm* aworm) {
      // structers are passed by value!
      // -> we return a copy here
      return aworm->wormpos[aworm->headindex];
      }

int getWormLength (struct worm* aworm) {
      return aworm->cur_lastindex+1;
} 