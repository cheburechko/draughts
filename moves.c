#include "board.h"
#include "moves.h"
#include "logics.h"
#include <stdlib.h>
#include <stdio.h>


//====================== Static functions =================================


// moves_state destructor
static void moves_destroy_state(moves_state * state)
{
	free(state -> params);
	if (state -> next)
		moves_destroy_moves(state -> next);
	state -> params = 0;
}

// creates a parameter array
static int * moves_create_params(int x, int y, int dx, int dy, int a,
									int move, rules_rules * rules)
{
	int * result;
	char type = rules -> moves[move].type;
	switch(type)
	{
		case 'm':
			result = (int *) malloc(sizeof(int) * 6);
			result[0] = x;
			result[1] = y;
			result[2] = x + dx * a;
			result[3] = y + dy * a;
			result[4] = a;
			result[5] = move;
			break;
		case 'j':
			result = (int *) malloc(sizeof(int) * 8);
			result[0] = x;
			result[1] = y;
			result[2] = x + dx * a;
			result[3] = y + dy * a;
			result[4] = a;
			result[5] = move;
			result[6] = result[2] - dx;
			result[7] = result[3] - dy;
			break;
	}
	return result;
}

// for current piece tries current move
// return action desciptor
static char * moves_try_move(board_play * play, int * params)
{
	char * result = 0;

	int i = 0;
	moves_move * move = &(play -> rules -> moves[params[5]]);

	// find a condition that fits this move and return a descriptor of
	// an action which is needed to be taken
	while (move -> conditions[i][0] != 0)
	{
		if (logics_check_condition(move -> conditions[i], play, params))
		{
			result = move -> actions[i];
			break;
		}
		i++;
	}
	return result;
}

// check if the already found moves fit the priority of the move that
// is gonna be added. If not, free the array and reset i
static void moves_priority_check(moves_state * array, int * i, int move,
								 rules_rules * rules)
{
	if (*i)
	if (rules -> moves[array[0].params[5]].priority <
		rules -> moves[move].priority)
	{
		int j;
		for (j = 0; j < *i; j++)
			moves_destroy_state(&(array[j]));
		*i = 0;
		array[0].params = 0;
	}
	return;
}

static int moves_find_longest(moves_state * array, board_play * play,
								int move, int *i)
{
	// make a move and look further
	int result = 1, j = 0;
	int player = play -> cur_player;

	moves_state * moves = moves_exec(play, &(array[*i]));

	if (play -> cur_player == player)
	{
		array[*i].next = moves;

		// check if there are any moves
		if (array[*i].next[0].params)
			array[*i].depth = array[*i].next[0].depth + 1;
	}
	else
		moves_destroy_moves(moves);

	// find out if the current move is the longest
	if (array[*i].depth > array[0].depth)
	{
		for (j = 0; j < *i; j++)
			moves_destroy_state(&(array[j]));
		array[0] = array[*i];
		*i = 0;
	}
	else if (array[*i].depth < array[0].depth)
	{
		moves_destroy_state(&(array[*i]));
		(*i)--;
		result = 0;
	}
	
	moves_unmake(play);
	return result;
}

// writes parameters in an array of moving in current direction
static int moves_write_direction(int x, int y, int dx, int dy, int any,
				int move, int * i, moves_state * array, board_play * play)
{
	int * params;
	char * action;
	int j, result = 0;
	char type = play -> rules -> moves[move].type;

	if (type == 'm')
		j = 1;
	else if (type == 'j')
		j = 2;

	if (any)
	{
		// compute obstacles
		int len = board_direction_length(play -> rules, x, y, dx, dy);
		board_obstacles * obs = board_get_obstacles(play, x, y, dx, dy);
		
		// try every move
		for ( ; j <= len; j++)
		{
			play -> obs = obs;
			params = moves_create_params(x, y, dx, dy, j, move,
										 play -> rules);
			action = moves_try_move(play, params);
			
			// if a move was successful, save it
			if (action)
				if (action[0] != ACT_ABORT)
				{
					moves_priority_check(array, i, move, play -> rules);
					array[*i].params = params;
					array[*i].action = action;
					array[*i].next = 0;
					array[*i].depth = 0;
					// if a move is repeatable and we need the
					// longest sequence, get it
					if (play -> rules -> moves[move].repeatable && 
						play -> rules -> longest_move)
						result += moves_find_longest(array, play, move, i);
					else
						result ++;
					(*i)++;
				}
				else
					free(params);
			else
				free(params);
		}
		board_destroy_obstacles(obs);
		play -> obs = 0;
	}
	else
		// check if the ending fit the board
		if(board_check_bounds(play -> rules, x + j*dx, y + j*dy))
		{
			params = moves_create_params(x, y, dx, dy, j, move, 
												play -> rules);
			action = moves_try_move(play, params);

			if (action)
				if (action[0] != ACT_ABORT)
				{
					moves_priority_check(array, i, move, play -> rules);
					array[*i].params = params;
					array[*i].action = action;
					array[*i].next = 0;
					array[*i].depth = 0;
					result = 1;
					if (play -> rules -> moves[move].repeatable && 
						play -> rules -> longest_move)
						result = moves_find_longest(array, play, move, i);
					(*i)++;
					return result;
				}
			free(params);
		}
	return result;
}

// puts all possible moves of current move type for current piece
// in an array. Returns priority if successful or -1 if not.
static int moves_possible_moves(board_play * play, int x, int y,
						int move_index, moves_state * array, int * l)
{
	if (!play)
		return;

	int added = 0;
	moves_move * move = &(play -> rules -> moves[move_index]);
	// Value of x and y offsets
	int dx = 0, dy = 0;
	// Just a counter
	int j = 0, i = 0;
	// Flags defining whether offsets can be used in both ways
	int idx = 0, idy = 0;
	// Flag that defines if a piece can choose any square in the
	// direction of the offset
	int any = 0;
	// Flag that defines a type of move
	char type = move -> type;
	//decrypt moves

	while (move -> dx[j])
	{
		switch(move -> dx[j])
		{
			// '-' shows negative values !must be put in the end!
			case '-':
				dx = -dx;
				break;
			// 'i' shows a piece can move forwards and backwards
			case 'i':
				idx = 1;
				break;
			// 'a' shows that a piece can move for any diastance
			case 'a':
				any = 1; dx = 1;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				dx = dx * 10 + move -> dx[j] - 48;
				break;
		}
		j++;
	}

	j = 0;

	// same here
	while (move -> dy[j])
	{
		switch(move -> dy[j])
		{
			case '-':
				dy = -dy;
				break;
			case 'i':
				idy = 1;
				break;
			case 'a':
				any = 1; dy = 1;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				dy = dy * 10 + move -> dy[j] - 48;
				break;
		}
		j++;
	}
	
	added += moves_write_direction(x, y, dx, dy, any, move_index, l, 
														array, play);
	if (idx)
		added += moves_write_direction(x, y, -dx, dy, any, move_index, l,
															array, play);
	if (idy)
	{
		added += moves_write_direction(x, y, dx, -dy, any, move_index, l,
															array, play);
		if (idy)
			added += moves_write_direction(x, y, -dx, -dy, any, move_index,
														l, array, play);
	}

	// check l
	if (*l >= play -> rules -> max_avail_moves)
		abort();

	if (added)
		return move -> priority;

	return -1;
}

// OBSOLETE
/* Performs a move. Returns:
 *	0 if a move is illegal
 *	1 if a move was successfully done
 *	2 returns a move if a piece should repeat the move again
 *
static int moves_perform_move(board_play * play, int * params)
{
	int result = 0;
	char * action = moves_try_move(play, params);
	if (action)
		result = logics_perform_action(action, play, params);
	return result;
}
*/



//======================= Public functions ================================





// get all available moves for a player
moves_state * moves_get(board_play * play)
{
	int x, y, move, i;
	int piece;
	int res;
	int player = play -> cur_player;
	moves_state * result = (moves_state *) malloc(
			sizeof(moves_state) * play -> rules -> max_avail_moves);
	int l = 0;
	int priority = 0;

	// check whether the move should be repeated
	board_play_state * last_move = &(play -> history[play -> cur_move - 1]);
	if (last_move -> player == play -> cur_player)
	{
		move = 0;
		piece = play -> board[last_move -> params[2]][last_move -> params[3]];
		while (play -> rules -> moves
				[play -> rules -> pieces_moves[piece][move]].priority ==
				play -> rules -> moves[last_move -> params[5]].priority)
		{
			moves_possible_moves(play, last_move -> params[2],
					last_move -> params[3],
					play -> rules -> pieces_moves[piece][move], result, &l);
			move++;
		}
	}
	else
	{
		// get all the moves
		for (i = play -> player_count[player] - 1; i >= 0; i--)
		{
			x = play -> piece_list[player * 2][i];
			y = play -> piece_list[player * 2 + 1][i];
			piece = play -> board[x][y];
			move = 0;
			while (play -> rules -> 
				pieces_moves[piece][move] != EMPTY &&
						play -> rules -> moves[
						play -> rules -> pieces_moves[piece][move]].priority
						>= priority)
			{
				res = moves_possible_moves(play, x, y, 
						play -> rules -> pieces_moves[piece][move], 
						result, &l);
				if (res > priority)
					priority = res;
				move++;
			}
		}
	}


	result[l].params = 0;
	return result;
}

// check events
void moves_check_events(board_play * play)
{
	int i;
	for (i = 0; i < play -> rules -> event_count; i++)
		if (logics_check_condition(play -> rules -> events[i].con, play, 0))
			logics_perform_action(play -> rules -> events[i].act, play, 0);
}

// Execute a move. Return next avilable moves.
moves_state * moves_exec(board_play * play, moves_state * state)
{
	moves_state * result;
	// do actions with board
	logics_perform_action(state -> action, play, state -> params);
	play -> cur_move++;
	play -> history[play -> cur_move].changed_pieces = 0;
	play -> history_shift = -1;
	moves_check_events(play);
	play -> history_shift = 0;
	
	// if we need the longest move, check if there are any other
	if (play -> rules -> longest_move && state -> next)
	{
		result = state -> next;
		state -> next = 0;
	}
	else	
		result = moves_get(play);


	if (play -> cur_player == play -> history[play -> cur_move - 1].player
		&& result[0].params == 0)
	{
		play -> cur_player++;
		play -> cur_player %= 2;
		free(result);
		play -> history_shift = -1;
		moves_check_events(play);
		play -> history_shift = 0;
		result = moves_get(play);
	}


	return result;
}

// Performs a move defined through string input.
// Returns 0 if the move is not valid, 1 if move is successful,
// 2 if there are no possible moves
moves_state * moves_define_and_make(board_play * play, char * input,
												moves_state * moves)
{
	int params[8];
	int i = 0, j = 0, out = 0;
	moves_state * result = 0;

	// decrypting input
	while (i < 4 && input[j] != 0)
	{
		if (input[j] <= 'z' && input[j] >= 'a')
		{
			params[i] = input[j] - 'a';
			i++;
		}
		else if (input[j] <= '9' && input[j] >= '0')
			 out = out * 10 + input[j] - '0';
		else if (out)
		{
			params[i] = out - 1;
			out = 0;
			i++;
		}
		j++;
	}

	if (out)
	{
		params[i] = out - 1;
		i++;
	}
	// not enough parameters
	if (i < 4) 
		return moves;
	
	i = 0;
	// search for our move
	while (moves[i].params && !result)
	{
		if (moves[i].params[0] == params[0] &&
			moves[i].params[1] == params[1] &&
			moves[i].params[2] == params[2] &&
			moves[i].params[3] == params[3])
			result = moves_exec(play, &(moves[i]));
		i++;
	}

	// if a move was executed, destroy last moves_state array
	// and return a new one
	if (result)
	{
		moves_destroy_moves(moves);
		return result;
	}
	return moves;
}

void moves_destroy_moves(moves_state * moves)
{
	if (!moves)
		return;
	//moves_debug_id++;
	//moves_debug();
	int i = 0;
	while (moves[i].params)
	{
		moves_destroy_state(&(moves[i]));
		i++;
	}
	free(moves);
}

void moves_debug(board_play * play)
{
	int i,j;
	for (i = play -> rules -> board_height - 1; i >= 0; i--)
	{
		for (j = 0; j < play -> rules -> board_width; j++)
			printf("%d  ", play -> index_board[i][j]);
		putc('\n', stdout);
	}
	printf("\n\n");
}

void moves_unmake(board_play * play)
{
	play -> cur_move--;
	play -> cur_player = play -> history[play -> cur_move].player;
	int i = 0;
	for (i = play -> history[play -> cur_move].changed_pieces - 1; i >= 0; i--)
	{
		//moves_debug(play);		
		int * placement = play -> history[play -> cur_move].placement[i];
		if (placement[2] == -1)
			board_move_piece(play, 
				play -> piece_list[placement[3] * 2][placement[4]],
				play -> piece_list[placement[3] * 2 + 1][placement[4]],
				play -> piece_list[placement[3] * 2][placement[4]]
						+ placement[0],
				play -> piece_list[placement[3] * 2 + 1][placement[4]] 
						+ placement[1], 0);
		else
			// place piece back at the end of the list
			board_replace_piece(play, placement[0], placement[1], 
								placement[2], 0, placement[4]);
	}
	play -> history[play -> cur_move].changed_pieces = 0;
}
