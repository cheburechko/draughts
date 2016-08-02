#ifndef __draughts__ai_h
#define __draughts__ai_h

#include "board.h"
#include "moves.h"

extern const infinitiy;

// Evaluation rules. Positive numbers good for white,
// negative for black
typedef struct ai_condition
{
	char condition[20];
	int weight;
	char piece;
} ai_condition;

// Bot player
typedef struct ai_bot
{
	int depth;
	int index;
} ai_bot;

extern int ai_debug, ai_max_depth;
extern int ai_processed_nodes;

// define default evaluation function
void ai_eval_default();

// destroy evaluation function
void ai_eval_destroy();

// Create a bot
ai_bot * ai_create_bot(int depth, int index);

// Destroy a bot
void ai_destoy_bot(ai_bot * bot);

// Function to make a move for bot
// return moves array
moves_state * ai_make_a_move(ai_bot * bot, board_play * play,
									moves_state * moves);

#endif
