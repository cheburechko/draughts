#ifndef __draughts_logics_h
#define __draughts_logics_h

#include "board.h"
#include "macros.h"

// Global macroses

/* Condition descriptors
#define CON_END 1
#define CON_START 2
#define CON_ENEMY 3
#define CON_DEFAULT 4
#define CON_EMPTY 5
#define CON_ALLY 6
#define CON_OBS 7
#define CON_COUNT 8
#define CON_PLAYER 9
#define CON_NO_MOVES 10
#define CON_PIECE 11
#define CON_COORD 12
#define CON_BOOL 13
#define CON_NUM 14
#define CON_TARGET 15
#define CON_ANY 16
#define CON_LINE 17

// Operator descriptors
#define OP_EQUAL '='
#define OP_INEQUAL '@'

// Action descriptors
#define ACT_REPLACE 1
#define ACT_ABORT 2
#define ACT_REMOVE_OBS 3
#define ACT_REPEAT 4
#define ACT_NEXT 5
#define ACT_CONVERT 6
*/


// State type
typedef struct logics_state {
	char * con;
	int pos;
	board_play * play;
	int * params;
} logics_state;

// functions that get values
typedef int * (* logics_eval) (logics_state * state);

// functions that perform actions
typedef int (* logics_act) (logics_state * state);

// Get the next value from a state
int * logics_get_value(logics_state * state);

// Checks a condition statement. Returns 0 if statement is false.
int logics_check_condition(char * con, board_play * play, int * params);

// Performs an action
int logics_perform_action(char * act, board_play * play, int * params);

// Get a pointer for logics var
int * logics_get_pointer(void);

#endif
