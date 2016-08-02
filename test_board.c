#include <assert.h>
#include <stdio.h>
#include "board.h"
#include "macros.h"
#include "rules.h"

int main()
{
	rules_rules * rules = rules_get_default();

	board_play * play = board_new_play(rules);
	//board_play * play2 = board_copy_play(play);

	assert(play -> board[0][0] == 0);
	assert(play -> board[4][6] == 2);
	assert(play -> board[3][3] == EMPTY);

	assert(play -> piece_count[0] == 12); 
	assert(play -> piece_count[1] == 0); 
	assert(play -> piece_count[2] == 12); 
	assert(play -> piece_count[3] == 0); 

	assert(play -> player_count[0] == 12); 
	assert(play -> player_count[0] == 12); 

	assert(play -> cur_player == 0);
/*
	assert(play2 -> board[0][0] == 0);
	assert(play2 -> board[4][6] == 2);
	assert(play2 -> board[3][3] == EMPTY);

	assert(play2 -> piece_count[0] == 12); 
	assert(play2 -> piece_count[1] == 0); 
	assert(play2 -> piece_count[2] == 12); 
	assert(play2 -> piece_count[3] == 0); 

	assert(play2 -> player_count[0] == 12); 
	assert(play2 -> player_count[0] == 12); 

	assert(play2 -> cur_player == 0);
	rules, 
*/
	// Check_bounds test
	assert(board_check_bounds(rules, 0, 0));
	assert(board_check_bounds(rules, 3, 3));
	assert(!board_check_bounds(rules, 2, 8));
	assert(!board_check_bounds(rules, 8, 5));
	assert(!board_check_bounds(rules, 9, 10));	

	// direction_length test
	assert(board_direction_length(rules, 0, 0, 1, 1) == 7);
	assert(board_direction_length(rules, 7, 7, 1, 1) == 0);
	assert(board_direction_length(rules, 7, 7, -1,-1) == 7);
	assert(board_direction_length(rules, 0, 0, -1, -1) == 0);
	assert(board_direction_length(rules, 0, 0, 1, 2) == 3);
	assert(board_direction_length(rules, 0, 0, 1, 0) == 7);
	assert(board_direction_length(rules, 0, 0, 0, 1) == 7);
	assert(board_direction_length(rules, 3, 3, -1, 0) == 3);

	// obstacles_test
	board_get_obstacles(play, 0, 0, 1, 1);
	assert(board_check_obstacles(rules, 2, 0) == 1);
	assert(board_check_obstacles(rules, 3, 0) == 2);
	assert(board_check_obstacles(rules, 7, 1) == 2);

	board_destroy_play(play);
	//board_destroy_play(play2);
	rules_destroy(rules);
	printf("Success!!!\n");
	return 0;
}
