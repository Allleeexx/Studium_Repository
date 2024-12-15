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






// *****************************************************
// Functions concerning the management of the worm data
// *****************************************************

// START WORM_DETAIL
// The following functions all depend on the model of the worm

// Getters
struct pos getWormHeadPos(struct worm* aworm) {
       // Structures are passed by value!
       // -> we return a copy here
       return aworm->wormpos[aworm->headindex];
}

// Initialize the worm
extern enum ResCodes initializeWorm(struct worm* aworm, int len_max, int len_cur, struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
    // Initialize last usable index to len_max-1
    // theworm_maxindex
    aworm->maxindex = len_max - 1;
    // Current last usable index in array. May grow upto maxindex
    aworm->cur_lastindex = len_cur - 1;

    // Initialize headindex
    // theworm_headindex
    aworm->headindex = 0;

    // Mark all elements as unused in the arrays of positions theworm_wormpos_y[] and theworm_wormpos_x[]
    // An unused position in the array is marked with code UNUSED_POS_ELEM
    for(int i = 0; i <= aworm->maxindex; i++) {
        aworm->wormpos[i].y = UNUSED_POS_ELEM;
        aworm->wormpos[i].x = UNUSED_POS_ELEM;
    }
    // Initialize position of worms head
    aworm->wormpos[aworm->headindex] = headpos;

    // Initialize the heading of the worm
    setWormHeading(aworm, dir);

    // Initialze color of the worm
    aworm->wcolor = color;

    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
extern void showWorm(struct board* aboard, struct worm* aworm) {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
    int tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex + 1);
    int i = aworm->headindex;
    do{
        if(aworm->wormpos[i].x == aworm->wormpos[tailindex].x && aworm->wormpos[i].y == aworm->wormpos[tailindex].y){
            placeItem(aboard,
            aworm->wormpos[i].y,
            aworm->wormpos[i].x,
            BC_USED_BY_WORM, SYMBOL_WORM_TAIL_ELEMENT, aworm->wcolor);
        } else if(aworm->wormpos[i].x == aworm->wormpos[aworm->headindex].x && aworm->wormpos[i].y == aworm->wormpos[aworm->headindex].y) {
            placeItem(aboard,
            aworm->wormpos[i].y,
            aworm->wormpos[i].x,
            BC_USED_BY_WORM, SYMBOL_WORM_HEAD_ELEMENT, aworm->wcolor);
        }else {
            placeItem(aboard,
            aworm->wormpos[i].y,
            aworm->wormpos[i].x,
            BC_USED_BY_WORM, SYMBOL_WORM_INNER_ELEMENT, aworm->wcolor);
        }
        i--;
        if(i < 0){ i = aworm->cur_lastindex; }
    } while(i != aworm->headindex && aworm->wormpos[i].x != UNUSED_POS_ELEM && aworm->wormpos[i].y != UNUSED_POS_ELEM);

    placeItem(aboard,
            aworm->wormpos[aworm->headindex].y,
            aworm->wormpos[aworm->headindex].x,
            BC_USED_BY_WORM, SYMBOL_WORM_HEAD_ELEMENT, aworm->wcolor);

}
void growWorm(struct worm* aworm, enum Boni growth) {
        // Play it safe and inhibit surpassing the bound
        if (aworm->cur_lastindex + growth <= aworm->maxindex) {
                aworm->cur_lastindex += growth;
        } else {
                aworm->cur_lastindex = aworm->maxindex;
        }
}

extern void cleanWormTail(struct board* aboard, struct worm* aworm) {
    int tailindex;

    // Compute tailindex
    tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex + 1);

    // Check the array of worm elements.
    // Is the array element at tailindex already in use?
    // Checking either array theworm_wormpos_y or theworm_wormpos_x is enough.
    if(aworm->wormpos[tailindex].x != UNUSED_POS_ELEM) {
        // YES: place a SYMBOL_FREE_CELL at the tail's position
        placeItem(aboard, aworm->wormpos[tailindex].y, aworm->wormpos[tailindex].x, BC_FREE_CELL, SYMBOL_FREE_CELL, aworm->wcolor);
    }
}

extern void moveWorm(struct board* aboard, struct worm* aworm, enum GameStates* agame_state) {
    struct pos headpos;
    // Get the current position of the worm's head element and compute the new head position according
    // to current heading. Do not store the new head position in the arrays of positions, yet.
    headpos = aworm->wormpos[aworm->headindex];
    // Compute new head position according to current heading.
    // Do not store the new head position in the array of positions, yet.
    headpos.x += aworm->dx;
    headpos.y += aworm->dy;

    // Check if we would hit something (for good or bad) or are going to leave the display if we move the
    // worm's head according to worm's last direction.
    // We are not allowed to leave the display's window.
    if (headpos.x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.x > getLastColOnBoard(aboard) ) {
        *agame_state = WORM_OUT_OF_BOUNDS;                  //@011 Anfang
    } else if (headpos.y < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
	} else if (headpos.y > getLastRowOnBoard(aboard) ) {
        *agame_state = WORM_OUT_OF_BOUNDS;                  //@011 Ende
    } else {
        // We will stay within bounds.
        // Check if the worm's head will colide with itself at the new position
        switch ( getContentAt(aboard, headpos) ) {
            case BC_FOOD_1:
                *agame_state = WORM_GAME_ONGOING;
                // Grow worm according to food item digested
                growWorm(aworm, BONUS_1);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_FOOD_2:
                *agame_state = WORM_GAME_ONGOING;
                growWorm(aworm, BONUS_2);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_FOOD_3:
                *agame_state = WORM_GAME_ONGOING;
                growWorm(aworm, BONUS_3);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_BARRIER:
                //That's bad: stop game
                *agame_state = WORM_CRASH;
                break;
            case BC_USED_BY_WORM:
                // Thats bad: stop game
                *agame_state = WORM_CROSSING;
                break;
            default:
                // Without default case we get a warning message.
                {;} // Do nothing. C syntax dictates some statement, here.
        }
    }
        // Check the status of *agame_state
        // Go on if nothing bad happened
        if ( *agame_state == WORM_GAME_ONGOING ) {
            // So all is well: we did not hit anything bad and did not leave the window. --> Update the worm structure.
            // Increment headindex
            // Go round if end of worm is reached (ring buffer)
            aworm->headindex++;
            if (aworm->headindex > aworm->cur_lastindex) {
                aworm->headindex = 0;
            }
            // Store new coordinates of head element in worm structure
            aworm->wormpos[aworm->headindex] = headpos;
    }
}





// A simple collision detection
extern bool isInUseByWorm(struct worm* aworm, struct pos new_headpos) {
    int i = aworm->headindex;
    bool collision = false;

    do {
        // Compare the position of the current worm element with the new_headpos_*
        if(new_headpos.y == aworm->wormpos[i].y && new_headpos.x == aworm->wormpos[i].x) {
            collision = true;
        }
        i = (i+1) % WORM_LENGTH;
    } while (i != aworm->headindex && aworm->wormpos[i].x != UNUSED_POS_ELEM);    // Solange i ungleich theworm_headindex UND Koordinate an Stelle i nicht unused

    // Return what we found out
    return collision;
}

// Setters
extern void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :// User wants up
            aworm->dx=0;
            aworm->dy=-1;
            break;
        case WORM_DOWN :// User wants down
            aworm->dx=0;
            aworm->dy=1;
            break;
        case WORM_LEFT :// User wants left
            aworm->dx=-1;
            aworm->dy=0;
            break;
        case WORM_RIGHT :// User wants right
            aworm->dx=1;
            aworm->dy=0;
            break;
    }
}

int getWormLength(struct worm* aworm) {
        return aworm->cur_lastindex + 1;
}
