#ifndef __draughts_board_h
#define __draughts_board_h

#include "macros.h"
#include "rules.h"

// Gloabal parameters


// Square type
typedef char board_square;

// Board type
typedef board_square ** board_board;

typedef struct board_obstacles
{
	int * obs;
	int pos;
	int players[3];
} board_obstacles;

// State type
typedef struct board_play_state
{
	int params[6];
	int placement[20][5];
	int changed_pieces;
	int player;
} board_play_state;

// Struct of the game process
typedef struct board_play
{
	// Board
	board_board board;
	// Current player descriptor
	int cur_player;
	// Some counters
	int counters[10];
	// Number of pieces of each type
	int * piece_count;
	// Piece list for each player
	int * piece_list[6];
	// Table of indexes of each piece
	int ** index_board;
	// Number of pieces belonging to each player
	int player_count[3];
	// Keeps the history of moves
	board_play_state history[1000];
	int cur_move;
	int history_shift;
	// Obstacles struct for computing
	board_obstacles * obs;
	// A pointer to rules
	rules_rules * rules;
} board_play;

int max(int a, int b);

int min(int a, int b);

// Check whether (x, y) lie on the board
int board_check_bounds(rules_rules * rules, int x, int y);


// Return the length of the line in the given direction on the board
int board_direction_length(rules_rules * rules, int x, int y, int dx, int dy);

// Compute obstacles in a given direction from the given location
// on the board
board_obstacles * board_get_obstacles(board_play * play, int x, int y, 
									  int dx, int dy);

// Check obstalces for a current length of the move for pre-computed
// direction
int board_check_obstalces(board_play * play, int pos, 
										int player);

// Destroy obstacles struct
void board_destroy_obstacles(board_obstacles * obs);

// Create and initiallize a new game
board_play * board_new_play(rules_rules * rules);

// Copy game process
//board_play * board_copy_play(board_play * play);

// Destroy play
void board_destroy_play(board_play * play);

// Add entry to history
void board_add_history_placement(board_play * play, int x, int y,
								 int piece, int player, int index);

// move piece from (x1, y1) to (x2, y2)
// write to history if history_flag is set
void board_move_piece(board_play * play, int x1, int y1, int x2, int y2,
					  int history_flag);

// replace square at (x, y) with piece and write it to piece_list at
// index if index not equal to -1, or at the end otherwise
// write to history if history_flag is set
void board_replace_piece(board_play * play, int x, int y, char piece,
						 int history_flag, int index);
#endif
