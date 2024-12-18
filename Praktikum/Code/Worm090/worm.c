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
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "prep.h"
#include "messages.h"
#include "worm.h"
#include "worm_model.h"
#include "board_model.h"
#include "options.h"

// ********************************************************************************************
// Forward declarations of functions
// ********************************************************************************************
// This avoids problems with the sequence of function declarations inside the code.
// Note: this kind of problem is solved by header files later on!

// Management of the game
void initializeColors();
void readUserInput(struct worm* aworm, enum GameStates* agame_state );
enum ResCodes doLevel(struct game_options* somegops, enum GameStates* agame_state, char* level_filename);


// Initialize colors of the game
void initializeColors() {
    // Define colors of the game
    start_color();      //Ermögliche Verwendung von Farbpaaren
    init_pair(COLP_USER_WORM,     COLOR_YELLOW,       COLOR_BLACK);    // Die 1. Farbe ist grün, die 2. schwarz
    init_pair(COLP_FREE_CELL,     COLOR_BLACK,      COLOR_BLACK);    // Beide Farben sind schwarz -> nicht sichtbar
    init_pair(COLP_FOOD_1,        COLOR_YELLOW,     COLOR_BLACK);
    init_pair(COLP_FOOD_2,        COLOR_MAGENTA,    COLOR_BLACK);
    init_pair(COLP_FOOD_3,        COLOR_CYAN,       COLOR_BLACK);
    init_pair(COLP_BARRIER,       COLOR_RED,       COLOR_BLACK);
}

void readUserInput(struct worm* aworm, enum GameStates* agame_state ) {
    int ch; // For storing the key codes

    if ((ch = getch()) > 0) {
        // Is there some user input?
        // Blocking or non-blocking depends of config of getch
        switch(ch) {
            case 'q' :    // User wants to end the show
                *agame_state = WORM_GAME_QUIT;
                break;
            case KEY_UP :// User wants up
                setWormHeading(aworm, WORM_UP);
                break;
            case KEY_DOWN :// User wants down
                setWormHeading(aworm, WORM_DOWN);              //@012 Anfang
                break;
            case KEY_LEFT :// User wants left
                setWormHeading(aworm, WORM_LEFT);
                break;
            case KEY_RIGHT :// User wants right
                setWormHeading(aworm, WORM_RIGHT);             //@012 Ende
                break;
            case 's' : // User wants single step
                nodelay(stdscr, FALSE);  //Make getch blocking
                break;
            case ' ' : // Terminate single step; make getch non-blocking again
                nodelay(stdscr, TRUE);   // Make getch non-blocking again
                break;
            case 'g' : // For development: let the worm grow by BONUS_§ elements
                growWorm(aworm, BONUS_3);
                break;
        }
    }
    return;
}

enum ResCodes doLevel(struct game_options* somegops, enum GameStates* agame_state, char* level_filename) {
    struct worm userworm; // Local variable for storing the user's worm
    struct board theboard; // Our game board
    /*enum GameStates game_state; // The current game_state            ---- Hier in Aufgabe 7 nach playgame verschoben*/
    enum ResCodes res_code; // Result code from functions
    bool end_level_loop;    // Indicates whether we should leave the main loop

    struct pos bottomLeft;   // Start positions of the worm

    /*
    // At the beginnung of the level, we still have a chance to win
    game_state = WORM_GAME_ONGOING;                                     ---- Hier den Teil in Aufgabe 7 in playgame verschoben*/

    // Setup the board
    res_code = initializeBoard(&theboard);
    if ( res_code != RES_OK) {
        return res_code;
    }

    // Initialize the current level
    res_code = initializeLevelFromFile(&theboard, level_filename);
    if ( res_code != RES_OK) {
        return res_code;
    }

    // There is always an initialized user worm.
    // Initialize the userworm with its size, position, heading.
    bottomLeft.y =  getLastRowOnBoard(&theboard);
    bottomLeft.x =  0;

    res_code = initializeWorm(&userworm, WORM_LENGTH, WORM_INITIAL_LENGTH, bottomLeft, WORM_RIGHT, COLP_USER_WORM);
    if ( res_code != RES_OK) {
        return res_code;
    }

    // Show worm at its initial position
    showWorm(&theboard, &userworm);

    // Display all what we have set up until now
    refresh();

    // Start the loop for this level
    end_level_loop = false; // Flag for controlling the main loop
    while(!end_level_loop) {    //entspricht while(end_level_loop == false)
        // Process optional user input
        readUserInput(&userworm, *agame_state);     //Wenn getch non-blocking und kein User-Input vorhanden ist, wird geskippt -> &game_state wird übergeben, um z.B. 'q' zu WORM_GAME_QUIT zu verarbeiten
        if (*agame_state == WORM_GAME_QUIT ) {
            end_level_loop = true;      //@014 Bedingung für Schleifen-Abbruch
            continue; // Go to beginning of the loop's block and check loop condition
        }

        // Process userworm
        // Clean the tail of the worm
        cleanWormTail(&theboard, &userworm);
        // Now move the worm for one step
        moveWorm(&theboard, &userworm, *agame_state);       //@015 &game_state ist als Argument in moveWorm nötig, um WORM_OUT_OF_BOUNDS zurückzugeben, falls der Wurm den Bildschirm verlässt
        // Bail out of the loop if something bad happened
        if ( *agame_state != WORM_GAME_ONGOING ) {
            end_level_loop = true;      //@016 Bedingung für Schleifen-Abbruch
            continue; // Go to beginning of the loop's block and check loop condition
        }
        // Show the worm at its new position
        showWorm(&theboard, &userworm);
        // END process userworm
        // Inform user about position and length of userworm in status window
        showStatus(&theboard, &userworm);
        // Sleep a bit before we show the updated window
        napms(somegops->nap_time);

        // Display all the updates
        refresh();
        // Are we done with that level?
        if (getNumberOfFoodItems(&theboard) == 0) {
            end_level_loop = true;
        }
        // Start next iteration
    }

    // Preset res_code for rest of the function
    res_code = RES_OK;

    // For some reason we left the control loop of the current level.
    // Check why according to game_state
    switch (*agame_state) {
        case WORM_GAME_ONGOING:
        if (getNumberOfFoodItems(&theboard) == 0) {
            showDialog("Sie haben diese Runde erfolgreich beendet !!!",
            "Bitte Taste druecken");
        } else {
            showDialog("Interner Fehler!","Bitte Taste druecken");
            // Correct result code
            res_code = RES_INTERNAL_ERROR;
        }
            break;
        case WORM_GAME_QUIT:
            // User must have typed 'q' for quit
            showDialog("Sie haben die aktuelle Runde abgebrochen!",
                       "Bitte Taste druecken");
            break;
        case WORM_CRASH:
            showDialog("Sie haben das Spiel verloren, weil Sie das Spielfeld verlassen haben.",
                       "Bitte Taste druecken");
            break;
        case WORM_OUT_OF_BOUNDS:
            showDialog("Sie haben das Spiel verloren, weil Sie das Spielfeld verlassen haben.",
                       "Bitte Taste druecken");
            break;
        default:
            showDialog("Interner Fehler!","Bitte Taste druecken");
            // Set error result code. This should never happen.
            res_code = RES_INTERNAL_ERROR;
}
    // However, in this version we do not yet check for the reason.
    // There is no user feedback at the moment!

    cleanupBoard(&theboard);
    // Normal exit point
    return res_code;        //@017 Rückgabe des zuvor gesetzten res_code
}


// playGame
// ********************************************************************************************

enum ResCodes playGame(int argc, char* argv[]) {
    enum ResCodes res_code; // Result code from functions
    struct game_options thegops; // For options passed on the command line
    enum GameStates game_state;

    game_state = WORM_GAME_ONGOING;
    // Read the command line options
    res_code = readCommandLineOptions(&thegops, argc, argv);
    if ( res_code != RES_OK) {
        return res_code; // Error: leave early
    }

    if (thegops.start_single_step) {
        nodelay(stdscr, FALSE); // make getch to be a blocking call
    }
    //Play the game
    if(thegops.start_level_filename != NULL){
        res_code = doLevel(&thegops, &game_state, thegops.start_level_filename);     //Hier noch game_state hinzugefügt in Aufgabe 7 in else auch
        free(thegops.start_level_filename);
    }else{
        res_code = doLevel(&thegops, &game_state ,"basic.level.1");
    }
    return res_code;
}

// MAIN
// ********************************************************************************************

int main(int argc, char* argv[]) {
    enum ResCodes res_code;         // Result code from functions

    // Here we start
    initializeCursesApplication();  // Init various settings of our application
    initializeColors();             // Init colors used in the game

    // Maximal LINES and COLS are set by curses for the current window size.
    // Note: we do not cope with resizing in this simple examples!

    // Check if the window is large enough to display messages in the message area
    // a has space for at least one line for the worm
    if ( LINES < ROWS_RESERVED + MIN_NUMBER_OF_ROWS || COLS < MIN_NUMBER_OF_COLS ) {
        // Since we not even have the space for displaying messages
        // we print a conventional error message via printf after
        // the call of cleanupCursesApp()
        cleanupCursesApp();
        printf("Das Fenster ist zu klein: wir brauchen mindestens %dx%d\n",
                MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS + ROWS_RESERVED );
        res_code = RES_FAILED;
    } else {
        res_code = playGame(argc, argv);
        cleanupCursesApp();
    }

    return res_code;    //@001 Rückgabe des zuvor gesetzten res_code
}
