#ifndef __draughts_moves_h
#define __draughts_moves_h

#include "board.h"

// Special type that describes the move that can be performed
typedef struct moves_state {
	int * params;
	char * action;
	struct moves_state * next;
	int depth;
} moves_state;

/* Checks if the game is over, returns:
   -1: game is not over
   0: a draw
   else number of a winner
*/
int rules_check_game_over(board_play * play);

// check events
void moves_check_events(board_play * play);

// Get an array of available moves for a player
moves_state * moves_get(board_play * play);

// Make a move based on the input. Return next available moves
moves_state *  moves_define_and_make(board_play * play, char * input,
												moves_state * moves);

// Execute a move. Return next available moves
moves_state *  moves_exec(board_play * play, moves_state * state);

// Destroy moves_state array
void moves_destroy_moves(moves_state * moves);

// Unmake last move
void moves_unmake(board_play * play);

#endif
