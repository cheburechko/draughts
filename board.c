#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "rules.h"
#include "board.h"

int max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

int min(int a, int b)
{
	if (a < b) return a;
	return b;
}

//checks if x and y lie on the board
int board_check_bounds(rules_rules * rules, int x, int y)
{
	if (x < 0 || y < 0 || x >= rules -> board_width 
					   || y >= rules -> board_height)
		return 0;
	return 1;
}


// return an array of possible moves of current type in current direction
int board_direction_length(rules_rules * rules, 
							int x, int y, int dx, int dy)
{
	int result = rules -> board_width + rules -> board_height;

	if (dx > 0)
		result = (rules -> board_width - x - 1) / dx;
	else if (dx < 0)
		result = x / (-dx);

	if (dy > 0)
		result = min(result, (rules -> board_height - y - 1) / dy);
	else if (dy < 0)
		result = min(result, y / (-dy));

	return result;
}

// count obstacles from a location in a direction defined by dx dy
board_obstacles * board_get_obstacles(board_play * play, int x, int y,
									  int dx, int dy)
{
	board_obstacles * result = (board_obstacles *) calloc
											(1, sizeof(board_obstacles));
	int i;
	i = board_direction_length(play -> rules, x, y, dx, dy);
	result -> obs = (int *) calloc(i + 1, sizeof(int));
	result -> obs[0] = i;

	for (i = 1; i <= result -> obs[0]; i++)
		result -> obs[i] = play -> board[x + i * dx][y + i * dy];

	// reset counters
	result -> pos = 1;

	return result;
}

// count obstacles of a specific player
int board_check_obstacles(board_play * play, int pos, int player)
{
	int p = 0;
	// update counters
	while (play -> obs -> pos < pos)
	{
		p = rules_get_player(play -> rules, 
				play -> obs -> obs[play -> obs -> pos]);
		if (p != EMPTY)
			play -> obs -> players[p]++;
		play -> obs -> pos++;
	}
	while (play -> obs -> pos > pos)
	{
		play -> obs -> pos--;
		p = rules_get_player(play -> rules, 
				play -> obs -> obs[play -> obs -> pos]);
		if (p != EMPTY)
			play -> obs -> players[p]++;
	}
	return play -> obs -> players[player];
}

void board_destroy_obstacles(board_obstacles * obs)
{
	free(obs -> obs);
	free(obs);
}
// copy board
static board_board board_copy_board(rules_rules * rules, board_board board)
{
	board_board result = (board_board) malloc
				(sizeof(board_square *) * rules -> board_width);
	int i;

	for (i = 0; i < rules -> board_width; i++)
	{
		result[i] = (board_square *) malloc
				(sizeof(board_square) * (rules -> board_height));
		memcpy((char *) result[i], (char *) board[i], 
						sizeof(board_square) * (rules -> board_height));
	}

	return result;
}

// copy play
/*
board_play * board_copy_play(board_play * play)
{
	board_play * result = (board_play *) malloc(sizeof(board_play));
	result -> board = board_copy_board(play -> rules, play -> board);
	result -> cur_player = play -> cur_player;
	int i;

	for (i = 0; i < 10; i++)
		result -> counters[i] = play -> counters[i];
	for (i = 0; i < 2; i++)
		result -> player_count[i] = play -> player_count[i];

	result -> piece_count = (int *) malloc(sizeof(int) * 
							 play -> rules -> pieces);
	for (i = 0; i < 4; i++)
		result -> piece_count[i] = play -> piece_count[i];
	result -> rules = play -> rules;

	result -> nplayer = play -> last_move.player;
	for (i = 0; i < 6; i++)
		result -> last_move.params[i] = play -> last_move.params[i];
	return result;
}
*/

// board
// create and initialize a new board
board_play * board_new_play(rules_rules * rules)
{
	board_play * result = (board_play *) malloc(sizeof(board_play));

	result -> cur_player = 0;
	
	result -> piece_count = (int * )malloc(sizeof(int) * rules -> pieces);
	int i,j;
	for (i = 0; i < rules -> pieces; i++)
		result -> piece_count[i] = rules -> piece_count[i];
		

	result -> board = (board_board) 
				malloc(sizeof(board_square *) * rules -> board_width);
	result -> index_board = (int **)
				malloc(sizeof(int *) * rules -> board_width);

	for (i = 0; i < rules -> board_width; i++)
	{
		result -> board[i] = (board_square *)
			malloc(sizeof(board_square) * (rules -> board_height));
		memcpy(result -> board[i], rules -> sample[i], 
					sizeof(board_square) * (rules -> board_height));

		result -> index_board[i] = (int *)
			malloc(sizeof(int) * rules -> board_height);
		for(j = 0; j < rules -> board_height; j++)
			result -> index_board[i][j] = 
				rules -> default_index_board[i][j];
	}

	//==============================================================
	for (i = 0; i < 3; i++)
	{
		result -> player_count[i] = rules -> player_count[i];
		result -> piece_list[i * 2] = calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));
		result -> piece_list[i * 2 + 1] = calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));

		for (j = 0; j < rules -> player_count[i]; j++)
		{
			result -> piece_list[i * 2][j] = 
					rules -> default_piece_list[i * 2][j];
			result -> piece_list[i * 2 + 1][j] = 
					rules -> default_piece_list[i * 2 + 1][j];
		}
	}

	result -> cur_move = 1;
	result -> history[0].player = 1;
	result -> history[0].changed_pieces = 0;
	result -> history[1].changed_pieces = 0;
	result -> history_shift = 0;

	for (i = 0; i < 6; i++)
		result -> history[0].params[i] = -1;
	
	result -> obs = 0;
	result -> rules = rules;
	return result;
}

// destroy a board
void board_destroy_board(rules_rules * rules, board_board board)
{
	int i;
	for (i = 0; i < rules -> board_width; i++)
		free(board[i]);
	free(board);
}

// destroy a play
void board_destroy_play(board_play * play)
{
	int i;
	free(play -> piece_count);

	for (i = 0; i < 6; i++)
		free(play -> piece_list[i]);

	board_destroy_board(play -> rules, play -> board);

	for (i = 0; i < play -> rules -> board_width; i++)
	{
		free(play -> index_board[i]);
	}
	free(play -> index_board);

	free(play);
	return;
}

// write changes to history in format (x, y, 
// piece:
// 0 - EMPTY - deleted piece
// -1 - moved, -2 - placed
// player, index
void board_add_history_placement(board_play * play, int x, int y, 
								 int piece, int player, int index)
{
	board_play_state * state = &(play -> history
			[play -> cur_move + play -> history_shift]);
	state -> placement[state -> changed_pieces][0] = x; 
	state -> placement[state -> changed_pieces][1] = y;
	state -> placement[state -> changed_pieces][2] = piece;
	state -> placement[state -> changed_pieces][3] = player;
	state -> placement[state -> changed_pieces][4] = index;
	state -> changed_pieces++;
}

// replace a square at (x,y) with a piece
// if index is -1, the square index should be created
// otherwise it should be replaced with index
void board_replace_piece(board_play * play, int x, int y, char piece, 
						 int history_flag, int index)
{
	int player = rules_get_player(play -> rules, play -> board[x][y]);
	int new_player = rules_get_player(play -> rules, piece);
	if (history_flag)
		board_add_history_placement(play, x, y, play -> board[x][y],
									player, play -> index_board[x][y]);

	// delete old index if neccessary
	if (player != EMPTY && player != new_player)
	{
		play -> player_count[player]--;
		// move last entry to the location of deleted one
		play -> piece_list[player * 2][play -> index_board[x][y]] = 
			play -> piece_list[player * 2][play -> player_count[player]];
		play -> piece_list[player * 2 + 1][play -> index_board[x][y]] = 
			play -> piece_list[player*2+1][play -> player_count[player]];
		// rewrite index of the moved entry
		play -> index_board
			[play -> piece_list[player * 2][play -> player_count[player]]]
			[play -> piece_list[player*2+1][play -> player_count[player]]]
			=  play -> index_board[x][y];
	}

	if (new_player != EMPTY)
	{
		if (index == -1 && player != new_player)
		{
			// add a new entry to the list
			play -> piece_list[new_player * 2]
							  [play -> player_count[new_player]] = x;
			play -> piece_list[new_player * 2 + 1]
							  [play -> player_count[new_player]] = y;
			// set the index
			play -> index_board[x][y] = play -> player_count[new_player]++;
		}
		else if (player != new_player)
		{
			// place piece at index at the end of the list
			if (index != play -> player_count[new_player])
			{
				play -> piece_list
					[new_player * 2][play -> player_count[new_player]] =
					play -> piece_list[new_player * 2][index];
				play -> piece_list
					[new_player * 2 + 1][play -> player_count[new_player]] =
					play -> piece_list[new_player * 2 + 1][index];
				play -> index_board
					[play -> piece_list[new_player * 2][index]]
					[play -> piece_list[new_player * 2 + 1][index]] = 
					play -> player_count[new_player];
			}
			play -> player_count[new_player]++;
			// place new piece at the index
			play -> piece_list[new_player * 2][index] = x;
			play -> piece_list[new_player * 2 + 1][index] = y;
			play -> index_board[x][y] = index;
		}
	}
	
	if (player < 2)
		play -> piece_count[play -> board[x][y]]--;
	if (new_player < 2)
		play -> piece_count[piece]++;

	play -> board[x][y] = piece;
}

void board_move_piece(board_play * play, int x1, int y1, int x2, int y2,
					  int history_flag)
{
	int player1 = rules_get_player(play -> rules, play -> board[x1][y1]);
	int player2 = rules_get_player(play -> rules, play -> board[x2][y2]);

	if (player2 != EMPTY)
	{
		// save data about deleted piece
		if (history_flag)
			board_add_history_placement(play, x2, y2, play -> board[x2][y2],
									player2, play -> index_board[x2][y2]);
		play -> player_count[player2]--;
		if (player2 < 2)
			play -> piece_count[play -> board[x2][y2]]--;

		play -> piece_list[player2 * 2][play -> index_board[x2][y2]] = 
			play -> piece_list[player2 * 2][play -> player_count[player2]];
		play -> piece_list[player2 * 2 + 1][play -> index_board[x2][y2]] = 
			play -> piece_list[player2*2+1][play -> player_count[player2]];
	}

	// save data about moved piece
	if (history_flag)
		board_add_history_placement(play, x1 - x2, y1 - y2, -1, player1, 
								play -> index_board[x1][y1]);
	// change indexes
	play -> piece_list[player1 * 2][play -> index_board[x1][y1]] = x2;
	play -> piece_list[player1 * 2 + 1][play -> index_board[x1][y1]] = y2;
	play -> index_board[x2][y2] = play -> index_board[x1][y1];
	play -> board[x2][y2] = play -> board[x1][y1];
	play -> board[x1][y1] = EMPTY;
}

