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

#ifndef _WORM_MODEL_H
#define _WORM_MODEL_H

#include <curses.h>
#include "worm.h"

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

// Functions concerning the management of the worm data
extern enum ResCodes initializeWorm(int len_max, int headpos_y, int headpos_x, enum WormHeading dir, enum ColorPairs color);
extern void showWorm();
extern void cleanWormTail();
extern void moveWorm(enum GameStates* agame_state);
extern bool isInUseByWorm(int new_headpos_y, int new_heapos_x);

//Setters
extern void setWormHeading(enum WormHeading dir);

#endif  // #define _WORM_MODEL_H
