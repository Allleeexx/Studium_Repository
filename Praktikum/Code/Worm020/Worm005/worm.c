// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //zugriff auf bib stdbool,elemente true und false
#include <time.h>
#include <string.h>
#include <unistd.h>

// ********************************************************************************************
// Constants, data structures
// ********************************************************************************************

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

// Directions for the worm
enum WormHeading {
  WORM_UP,
  WORM_DOWN,
  WORM_LEFT,
  WORM_RIGHT,
  WORM_LEFT_UP,
  WORM_RIGHT_UP,
  WORM_LEFT_DOWN,
  WORM_RIGHT_DOWN,
};

// ********************************************************************************************
// Global variables
// ********************************************************************************************

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

// ********************************************************************************************
// Forward declarations of functions
// ********************************************************************************************
// This avoids problems with the sequence of function declarations inside the code.
// Note: this kind of problem is solved by header files later on!

// Management of the game
void initializeColors();
void readUserInput(enum GameStates* agame_state );
enum ResCodes doLevel();

// Standard curses initialization and cleanup
void initializeCursesApplication(); 
void cleanupCursesApp(void);

// Placing and removing items from the game board
// Check boundaries of game board
void placeItem(int y, int x, chtype symbol, enum ColorPairs color_pair);
int getLastRow();
int getLastCol();

// Functions concerning the management of the worm data
enum ResCodes initializeWorm(int len_max, int headpos_y, int headpos_x, enum WormHeading dir, enum ColorPairs color);
void showWorm();
void cleanWormTail();
void moveWorm(enum GameStates* agame_state);
bool isInUseByWorm(int new_headpos_y, int new_heapos_x);
void setWormHeading(enum WormHeading dir);

// ********************************************************************************************
// Functions
// ********************************************************************************************

// ************************************
// Management of the game
// ************************************

// Initialize colors of the game
void initializeColors() {
    // Define colors of the game
    start_color();
    init_pair(COLP_USER_WORM,    COLOR_GREEN /*@002*/,    COLOR_BLACK);
    init_pair(COLP_FREE_CELL,    COLOR_BLACK,             COLOR_BLACK);
}

void readUserInput(enum GameStates* agame_state ) {
    int ch; // For storing the key codes

    if ((ch = getch()) > 0) {
        // Is there some user input?
        // Blocking or non-blocking depends of config of getch
        switch(ch) {
            case 'q' :    // User wants to end the show
                *agame_state = WORM_GAME_QUIT;
                break;
            case KEY_UP :// User wants up
                setWormHeading(WORM_UP);
                break;
            case KEY_DOWN :// User wants down
                setWormHeading(WORM_DOWN); /*@012*/
                break;
            case KEY_LEFT :// User wants left
                setWormHeading(WORM_LEFT); /*@012*/
                break;
            case KEY_RIGHT :// User wants right
                setWormHeading(WORM_RIGHT); /*@012*/
                break;

            case 'w' : // schräg nach links oben
                setWormHeading(WORM_LEFT_UP);
                break;
            case 'd' : // schräg nach rechts oben
                setWormHeading(WORM_RIGHT_UP);
                break;
            case 'a' : // schräg links unten
                setWormHeading(WORM_LEFT_DOWN);
                break;
            case 'x' : // schräg unten rechts
                setWormHeading(WORM_RIGHT_DOWN);
                break;

            case 's' : // User wants single step
                nodelay(stdscr,FALSE); //@013  We simply make getch blocking
                break;
            case ' ' : // Terminate single step; make getch non-blocking again
                nodelay(stdscr,TRUE); // @013   Make getch non-blocking again
                break;
        }
    }
    return;
}

enum ResCodes doLevel() {
    enum GameStates game_state; // The current game_state

    enum ResCodes res_code; // Result code from functions
    bool end_level_loop;    // Indicates whether we should leave the main loop

    int bottomLeft_y, bottomLeft_x;   // Start positions of the worm

    // At the beginnung of the level, we still have a chance to win
    game_state = WORM_GAME_ONGOING;

    // There is always an initialized user worm.
    // Initialize the userworm with its size, position, heading.
    bottomLeft_y =  getLastRow();
    bottomLeft_x =  0;

    res_code = initializeWorm(WORM_LENGTH, bottomLeft_y, bottomLeft_x , WORM_RIGHT, COLP_USER_WORM);
    if ( res_code != RES_OK) {
        return res_code;
    }

    // Show worm at its initial position
    showWorm();


    // Display all what we have set up until now
    refresh();

    // Start the loop for this level
    end_level_loop = false; // Flag for controlling the main loop
    while(!end_level_loop) {        //solange es nicht gleich false ist(also true)
        // Process optional user input
        readUserInput(&game_state); 
        if ( game_state == WORM_GAME_QUIT ) {
            end_level_loop = true; // @014
            continue; // Go to beginning of the loop's block and check loop condition
        }

        // Process userworm
        // Clean the tail of the worm
        cleanWormTail();
        // Now move the worm for one step
        moveWorm(&game_state /*@015*/);
        // Bail out of the loop if something bad happened
        if ( game_state != WORM_GAME_ONGOING ) {
            end_level_loop = true; // @016;
            continue; // Go to beginning of the loop's block and check loop condition
        }
        // Show the worm at its new position
        showWorm();
        // END process userworm

        // Sleep a bit before we show the updated window
        napms(NAP_TIME);

        // Display all the updates
        refresh();

        // Start next iteration
    }

    // Preset res_code for rest of the function
    res_code = RES_OK;

    // For some reason we left the control loop of the current level.
    // However, in this version we do not yet check for the reason.
    // There is no user feedback at the moment!

    // Normal exit point
    return res_code; // @017
}

// *********************************************
// Standard curses initialization and cleanup
// *********************************************

// Initialize application with respect to curses settings
void initializeCursesApplication() {
    initscr(); // Initialize the curses screen

    // Note:
    // The call to initscr() defines various global variables of the curses framework.
    // stdscr, LINES, COLS, TRUE, FALSE

    noecho();  // Characters typed ar not echoed
    cbreak();  // No buffering of stdin
    nonl();    // Do not translate 'return key' on keyboard to newline character
    keypad(stdscr, TRUE); // Enable the keypad
    curs_set(0);          // Make cursor invisible
    // Begin in non-single-step mode (getch will not block)
    nodelay(stdscr, TRUE);  // make getch to be a non-blocking call
}

// Reset display to normale state and terminate curses application
void cleanupCursesApp(void)
{
    standend();   // Turn off all attributes
    refresh();    // Write changes to terminal
    curs_set(1);  // Set cursor state to normal visibility
    endwin();     // Terminate curses application
}

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
int getLastRow() {
    return (LINES-1); //@003;
}

// Get the last usable column on the display
int getLastCol() {
    return (COLS-1); // @004
}

// *****************************************************
// Functions concerning the management of the worm data
// *****************************************************

// START WORM_DETAIL
// The following functions all depend on the model of the worm

// Initialize the worm
enum ResCodes initializeWorm(int len_max /*maximale lange des Wurms*/, int headpos_y, int headpos_x, enum WormHeading dir, enum ColorPairs color) {
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
void showWorm() {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
    placeItem(
            theworm_wormpos_y[theworm_headindex] ,
            theworm_wormpos_x[theworm_headindex] /* @007*/,
            SYMBOL_WORM_INNER_ELEMENT,theworm_wcolor);
}

void cleanWormTail(){
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

void moveWorm(enum GameStates* agame_state) {
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

bool isInUseByWorm(int new_headpos_y, int new_headpos_x) {
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

// Setters
void setWormHeading(enum WormHeading dir) {
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

// END WORM_DETAIL
// ********************************************************************************************

// ********************************************************************************************
// MAIN
// ********************************************************************************************

int main(void) {
    int res_code;         // Result code from functions
    
    //printf("press key to continue\n");
    getchar(); // start pogramm, give debugger a chance to attach, waits for eingabe


    // Here we start
    initializeCursesApplication();  // Init various settings of our application
    initializeColors();             // Init colors used in the game

    // Maximal LINES and COLS are set by curses for the current window size.
    // Note: we do not cope with resizing in this simple examples!

    // Check if the window is large enough to display messages in the message area
    // a has space for at least one line for the worm
    if ( LINES < MIN_NUMBER_OF_ROWS || COLS < MIN_NUMBER_OF_COLS ) {
        // Since we not even have the space for displaying messages
        // we print a conventional error message via printf after
        // the call of cleanupCursesApp()
        cleanupCursesApp();
        printf("Das Fenster ist zu klein: wir brauchen mindestens %dx%d\n",
                MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS );
        res_code = RES_FAILED;
    } else {
        res_code = doLevel();
        cleanupCursesApp();
    }

    return res_code; //*@001
}