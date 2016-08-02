#include "board.h"
#include "logics.h"
#include "macros.h"
#include "rules.h"
#include <stdlib.h>

/*=================== Condition processor specifications ==================
	There are 2 main functions: rules_get_value and 
	rules_check_condition.

	rules_get_value:
	* takes a condition string, an index pointer,
	  a play pointer and the parameters pointer
	  (usual move params are {start_x, start_y, end_x, end_y, var_a, var_b, 
							target_x, target_y})
	  as parameters.
	* returns an int array pointer where the first element 
	  is value type descriptor (eg. CON_PLAYER) and the next 
	  elements define this value.
	* memory returned by the function lies in stack and does boe
	  need to be freed.

	rules_check_condition:
	* takes a condition string, a play pointer and parameters ptr
	  as parameters
	* returns 0 if condition is false and 1 otherwise.
	* condition string may contain condition descriptors,
	  digits and '=' operator
*/

//=================== Stack for logics vars ==============================

// vairables stack
static int logics_stack[30][3];
static int logics_stack_top = 0;

int * logics_get_pointer(void)
{
	return logics_stack[logics_stack_top++];
}

static void logics_free_stack(void)
{
	logics_stack_top = 0;
}


//========================= Static functions ==============================

//====================== CONDITIONS FUNCTIONS =============================

// count pieces of a current player
static int * logics_count_pieces(logics_state * state)
{
	int * result = logics_get_pointer();
	state -> pos++;
	int * next = logics_get_value(state);
	result[0] = CON_NUM;

	switch(next[0])
	{
		case CON_PLAYER:
			result[1] = state -> play -> player_count[next[1]];
			break;
		case CON_PIECE:
			result[1] = state -> play -> piece_count[next[1]];
			break;
		case CON_LINE:
			break;
	}

	return result;
}

// define a player for current parametres
static int * logics_define_player(logics_state * state)
{
	int * result = logics_get_pointer();
	int * next;
	result[0] = CON_PLAYER;

	switch(state -> con[state -> pos++])
	{
		case CON_ENEMY: //return an opponent of the current player
			result[1] = (state -> play -> rules -> 
				piece_owners[state -> play -> board
				[state -> params[0]][state -> params[1]]] + 1) % 2;
			break;

		case CON_ALLY:	//return an owner of the piece at the start:
			result[1] = state -> play -> rules -> 
				piece_owners[state -> play -> board
				[state -> params[0]][state -> params[1]]];
			break;

		case CON_PLAYER:  /* params:
				CON_COORD/END/START: defines which player owns a piece
									 at a given location
				'0'..'9': just a player
			*/
			next = logics_get_value(state);
			switch(next[0])
			{
				case CON_NUM:
					result[1] = next[1];
					break;

				case CON_COORD:
					result[1] = rules_get_player(state -> play -> rules,
							state -> play -> board[next[1]][next[2]]);
					break;
			}
			break;

		case CON_LAST_PLAYER:
			result[1] = state -> play -> history
					[state -> play -> cur_move - 1].player;
			break;
		case CON_CUR_PLAYER:
			result[1] = state -> play -> cur_player;
			break;

		case CON_EMPTY:	//return empty player
			result[1] = EMPTY;
			break;
	}
	return result;
}

// moves
// check if a player or a piece can move
static int * logics_no_moves(logics_state * state)
{
	state -> pos++;
	int * next = logics_get_value(state);
	int * result = logics_get_pointer();

	// you can move forever )))
	result[0] = CON_BOOL;
	result[0] = 0;

	return result;
}


// convert digital chars to ints
static int * logics_get_int(logics_state * state)
{
	int * result = logics_get_pointer();
	result[0] = CON_NUM;
	result[1] = 0;

	switch (state -> con[state -> pos++])
	{
		case CON_NUM:
			while (state -> con[state -> pos] <= '9' &&
				   state -> con[state -> pos] >= '0')
			{
				result[1] *= 10;
				result[1] += state -> con[state -> pos] - 48;
				state -> pos++;
			}
			break;

		case CON_BYTE:
			result[1] = state -> con[state -> pos++];
			break;
	}

	return result;
}

// get coordinates
static int * logics_get_coords(logics_state * state)
{
	int * result = logics_get_pointer();
	int * next;
	result[0] = CON_COORD;

	switch (state -> con[state -> pos++])
	{
		case CON_END: //return coordinates of the move destination
			result[1] = state -> params[2];
			result[2] = state -> params[3];
			break;

		case CON_START:	//return coordinates of the move start
			result[1] = state -> params[0];
			result[2] = state -> params[1];
			break;

		case CON_TARGET: //return coordinates of the target square
			result[1] = state -> params[6];
			result[2] = state -> params[7];
			break;

		case CON_COORD: //return next two nums as coordinates
			next = logics_get_value(state);
			result[1] = next[1];
			next = logics_get_value(state);
			result[2] = next[1];
			break;
	}
	return result;
}

// get boolean
static int * logics_get_boolean(logics_state * state)
{
	int * result = logics_get_pointer();
	int * next;
	result[0] = CON_BOOL;

	switch (state -> con[state -> pos++])
	{
		case CON_DEFAULT:	//return true
			result[1] = 1;
			break;

		case CON_BOOL:
			next = logics_get_value(state);
			result[1] = next[1];
			break;
	}

	return result;
}

// checks obstacles
static int * logics_check_obstacles(logics_state * state)
{
	state -> pos++;
	int * result = logics_get_pointer();
	result[0] = CON_NUM;
	int * next = logics_get_value(state);

	switch (next[0])
	{
		case CON_PLAYER:
			result[1] = board_check_obstacles(state -> play,
										state -> params[4], next[1]);
			break;
		case CON_ANY:
			result[1] = board_check_obstacles(state -> play,
											state -> params[4], 0) +
						board_check_obstacles(state -> play,
											state -> params[4], 1) +
						board_check_obstacles(state -> play,
											state -> params[4], 2);
			break;
	}

	return result;
}

// define a type of the piece
static int * logics_define_piece(logics_state * state)
{
	state -> pos++;
	int * result = logics_get_pointer();
	result[0] = CON_PIECE;
	int * next = logics_get_value(state);

	switch (next[0])
	{
		case CON_NUM:
			result[1] = next[1];
			break;

		case CON_COORD:
			result[1] = state -> play -> board[next[1]][next[2]];
			break;
	}

	return result;
}

// any var
static int * logics_any(logics_state * state)
{
	state -> pos++;
	int * result = logics_get_pointer();
	result[0] = CON_ANY;
	return result;
}

// compute the equation
static int * logics_compute(int * l, int * r, char op)
{
	int * result = logics_get_pointer();
	result[0] = CON_BOOL;

	switch(op)
	{
		case '=':
			result[1] = (l[0] == r[0]) && (l[1] == r[1]);
			if (l[0] == CON_COORD)
				result[1] = result[1] && (l[2] == r[2]);	
			break;

		case '@': //!=
			result[1] = (l[0] != r[0]) || (l[1] != r[1]);
			if (l[0] == CON_COORD)
				result[1] = result[1] || (l[2] != r[2]);
			break;
	}

	return result;
}

// get line
static int * logics_get_line(logics_state * state)
{
	state -> pos++;
	int * result = logics_get_pointer();
	result[0] = CON_LINE;
	int * num = logics_get_value(state);
	result[1] = num[1];

	return result;
}

static int * logics_get_counter(logics_state * state)
{
	return 0;
}

//==================== ACTION FUNCTIONS ===================================

// replace the end with the start and free the start
static int logics_replace(logics_state * state)
{
	board_move_piece(state -> play, state -> params[0], state -> params[1],
			state -> params[2], state -> params[3], 1);

	state -> pos++;

	return 1;
}

// abort the move
static int logics_abort(logics_state * state)
{
	state -> pos++;
	return 0;
}

// remove all obstacles between the end and the start
static int logics_remove_obstacles(logics_state * state)
{
	state -> pos++;
	int x = state -> params[0], y = state -> params[1];
	int dx = (state -> params[2] - x) / state -> params[4];
	int dy = (state -> params[3] - y) / state -> params[4];
	int * piece = logics_get_value(state);
	x += dx; y += dy;

	while (x != state -> params[2] || y != state -> params[3])
	{
		if (state -> play -> board[x][y] != EMPTY)
			board_replace_piece(state -> play, x, y, piece[1], 1, -1);
		x += dx; y += dy;
	}

	return 1;
}

// repeat the move
static int logics_repeat(logics_state * state)
{
	state -> play -> history[state -> play -> cur_move].player
										= state -> play -> cur_player;
	int i;
	for (i = 0; i < 6; i++)
		state -> play -> history[state -> play -> cur_move].params[i] 
													= state -> params[i];
	state -> pos++;
	return 1;
}

// finish the move and give it to the next player
static int logics_next_player(logics_state * state)
{
	logics_repeat(state);
	state -> play -> cur_player ++;
	state -> play -> cur_player %= 2;
	state -> pos++;
	return 1;
}

// convert pieces from location of 1st type to 2nd
static int logics_convert(logics_state * state)
{
	state -> pos++;
	int * location = logics_get_value(state);
	int * piece1 = logics_get_value(state);
	int * piece2 = logics_get_value(state);
	int i;

	switch (location[0])
	{
		case CON_LINE:
			for (i = 0; i < state -> play -> rules -> board_width; i++)
				if (state -> play -> board[i][location[1]] == piece1[1])
					board_replace_piece(state -> play, i, location[1], 
										piece2[1], 1, -1);
			break;
	}
	return 1;
}

static int logics_mod_counter(logics_state * state)
{
	return 1;
}

static int logics_remove(logics_state * state)
{
	state -> pos ++;
	int * params = logics_get_value(state);
	int i = 0, player = 0;
	switch (params[0])
	{
		case CON_PIECE:
			player = rules_get_player(state -> play -> rules, params[1]);
			if (player == EMPTY)
				break;
			i = 0;
			while(i < state -> play -> player_count[player])
				if (state -> play -> board
						[state -> play -> piece_list[player * 2][i]]
						[state -> play -> piece_list[player * 2 + 1][i]]
						== params[1])
					board_replace_piece(state -> play,
						state -> play -> piece_list[player * 2][i],
						state -> play -> piece_list[player * 2 + 1][i],
						EMPTY, 1, -1);
				else
					i++;
			break;
							
	}
}
//===================== Public functions ==================================



// An array of eval functions for each condition descriptor
logics_eval logics_con_eval[32] = {
	0, 							// null
	logics_get_coords, 			// CON_END
	logics_get_coords, 			// CON_START
	logics_define_player, 		// CON_ENEMY
	logics_get_boolean, 		// CON_DEFAULT
	logics_define_player, 		// CON_EMPTY
	logics_define_player, 		// CON_ALLY
	logics_check_obstacles, 	// CON_OBS
	logics_count_pieces, 		// CON_COUNT
	logics_define_player, 		// CON_PLAYER
	logics_no_moves, 			// CON_NO_MOVES
	logics_define_piece, 		// CON_PIECE
	logics_get_coords, 			// CON_COORDS
	logics_get_boolean, 		// CON_BOOL
	logics_get_int, 			// CON_NUM
	logics_get_coords, 			// CON_TARGET
	logics_any, 				// CON_ANY
	logics_get_line,			// CON_LINE
	logics_define_player,		// CON_LAST_PLAYER
	logics_define_player,		// CON_CUR_PLAYER
	logics_get_line,			// CON_COLUMN
	logics_get_line,			// CON_LR_DIAG
	logics_get_line,			// CON_RL_DIAG
	logics_get_counter,			// CON_COUNTER
	logics_get_coords,			// CON_OFFSET
	logics_get_int,				// CON_BYTE
};

// An array of functions that perform actions
logics_act logics_actions[16] = {
	0,							// null
	logics_replace,				// ACT_REPLACE
	logics_abort,				// ACT_ABORT
	logics_remove_obstacles,	// ACT_REMOVE_OBS
	logics_repeat,				// ACT_REPEAT
	logics_next_player,			// ACT_NEXT
	logics_convert,				// ACT_CONVERT
	logics_mod_counter,			// ACT_INC_COUNTER
	logics_mod_counter,			// ACT_DEC_COUNTER
	logics_remove,				// ACT_REMOVE
};

// get value of the first con character
int * logics_get_value(logics_state * state)
{
	return logics_con_eval[state -> con[state -> pos]](state);
}

// check con statement
int logics_check_condition(char * con, board_play * play, int * params)
{
	int * lvalue, *rvalue, *buf;
	int result;
	char operator;
	logics_state state = {
		.con = con,
		.play = play,
		.params = params,
		.pos = 0
	};

	lvalue = logics_get_value(&state);
	operator = state.con[state.pos++];
	while (operator != 0)
	{
		rvalue = logics_get_value(&state);
		buf = logics_compute(lvalue, rvalue, operator);
		lvalue = buf;
		operator = state.con[state.pos++];
	}
	
	result = lvalue[1];
	logics_free_stack();
	return result;
}

// perform an action
int logics_perform_action(char * act, board_play * play, int * params)
{
	int result = 1;
	logics_state state = {
		.con = act,
		.play = play,
		.params = params,
		.pos = 0
	};

	while (state.con[state.pos] != 0 && result == 1)
		result = logics_actions[state.con[state.pos]](&state);

	return result;
}

