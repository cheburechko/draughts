#ifndef __draughts_rules_h
#define __draughts_rules_h

#include "parser.h"

typedef struct rules_request
{
	void * pointer;
	enum dataType type;
	char * key;
	int child_request_index;
	int size;
} rules_request;

// Move type: 
typedef struct moves_move
{
	// Move type, x offset, y offset
	char type, priority;
	char dx[5],dy[5];
	// Flag
	char repeatable;
	// Conditions of the move, actions, notation to output the move
	char conditions[20][10], actions[20][10];
} moves_move;

// event struct
typedef struct rules_event {
	char con[20], act[20];
} rules_event;


// Rules struct
typedef struct rules_rules {
	// defines pieces count
	int pieces;
	//board parameters
	int board_height, board_width;
	char ** sample;
	// starting counts
	int player_count[3];
	int * piece_count;
	// defines graphical representation of each piece
	char * piece_icon;
	// defines a master player for every piece
	char * piece_owners;
	// default piece lists
	int * default_piece_list[6];
	int ** default_index_board;

	// defines  moves count
	int moves_count;
	// defines all possible moves in the game
	moves_move * moves;
	// defines which moves can be used by each piece
	int ** pieces_moves;
	// maximum available moves per halfturn
	int max_avail_moves;
	// flag that defines if the longest possible move must be done
	int longest_move;

	// defines when the game is over
	char * game_over_conditions;
	// defines a winner for every game over condition
	int * game_over_winner;

	// ammount of events
	int event_count;
	// events
	rules_event * events;

	// piece weight for AI
	int * ai_eval_piece_weight;
	// position weight for AI
	int ** ai_eval_board;
	// bots depths
	int ai_bots[3];
	// description
	char * description;
} rules_rules;

// set default rules
rules_rules * rules_get_default(void);

// establish new rules
rules_rules * rules_get(char * filepath);

// get player
int rules_get_player(rules_rules * rules, char piece);

void rules_destroy(rules_rules * rules);

#endif
