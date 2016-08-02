#include <assert.h>
#include <stdio.h>
#include "board.h"
#include "rules.h"
#include "macros.h"
#include "logics.h"

int main ()
{
	rules_rules * rules = rules_get_default();
	board_play * play = board_new_play(rules);
	
	// conditions test

	char conditions [25][15] = {
		{CON_END, OP_EQUAL, CON_COORD, CON_NUM, '5', CON_NUM, '5', 0},
		{CON_END, OP_INEQUAL, CON_COORD, CON_NUM, '5', CON_NUM, '4', 0},
		{CON_START, OP_EQUAL, CON_COORD, CON_NUM, '4', CON_NUM, '2', 0},
		{CON_START, OP_INEQUAL, CON_COORD, CON_NUM, '3', CON_NUM, '2', 0},

		{CON_PLAYER, CON_END, OP_EQUAL, CON_ENEMY, 0},
		{CON_PLAYER, CON_END, OP_INEQUAL, CON_EMPTY, 0},
		{CON_PLAYER, CON_START, OP_EQUAL, CON_ALLY, 0},
		{CON_PLAYER, CON_START, OP_INEQUAL, CON_ENEMY, 0},

		{CON_DEFAULT, OP_EQUAL, CON_BOOL, CON_NUM, '1', 0},
		{CON_DEFAULT, OP_INEQUAL, CON_BOOL, CON_NUM, '0', 0},

		{CON_PLAYER, CON_TARGET, OP_EQUAL, CON_EMPTY},
		{CON_PLAYER, CON_TARGET, OP_INEQUAL, CON_PLAYER, CON_NUM, '0', 0},

		{CON_OBS, CON_ENEMY, OP_EQUAL, CON_NUM, '1', 0},
		{CON_OBS, CON_ENEMY, OP_INEQUAL, CON_NUM, '2', 0},
		{CON_OBS, CON_ALLY, OP_EQUAL, CON_NUM, '2', 0},
		{CON_OBS, CON_ALLY, OP_INEQUAL, CON_NUM, '1', '0', 0},
		{CON_OBS, CON_ANY, OP_EQUAL, CON_NUM, '3', 0},
		{CON_OBS, CON_ANY, OP_INEQUAL, CON_NUM, '9', '9', '9', 0},

		{CON_COUNT, CON_ENEMY, OP_EQUAL, CON_NUM, '1', '2', 0},
		{CON_COUNT, CON_ALLY, OP_INEQUAL, CON_NUM, '1', '3', 0},
		 
		{CON_PIECE, CON_START, OP_EQUAL, CON_PIECE, CON_NUM, '0', 0},
		{CON_PIECE, CON_END, OP_EQUAL, CON_PIECE, CON_NUM, '2', 0},
		{CON_PIECE, CON_TARGET, OP_EQUAL, CON_PIECE, CON_NUM, EMPTY, 0},
		
		{CON_DEFAULT, 0},
		{CON_DEFAULT, 0}

	};
	int params[25][15] = {
		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},

		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},

		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},

		{4, 2, 5, 5, 3, 0, 1, 6},
		{4, 2, 5, 5, 3, 0, 1, 6},

		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},

		{2, 0, 7, 5, 5, 0, 6, 4},
		{7, 1, 2, 6, 5, 0, 0, 0},

		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},

		{6, 0, 0, 6, 6, 0, 0, 0},
		{6, 0, 0, 6, 6, 0, 0, 0},
	};
	int i;
	
	board_get_obstacles(play, 6, 0, -1, 1);

	for (i = 0; i < 15; i++)
	{
		assert(logics_check_condition(conditions[i], play, params[i]));
	}

	// actions test
	char actions[15][15] = {
		{ACT_REPLACE, 0},
		{ACT_ABORT, 0},
		{ACT_REMOVE_OBS, 0},
		{ACT_REPEAT, 0},

		{ACT_REPLACE, ACT_REPLACE, 0},

		{ACT_ABORT, ACT_REPLACE, 0},
		{ACT_REPEAT, 0},
		{ACT_REMOVE_OBS, ACT_REPLACE, ACT_REPEAT, 0}
	};

	assert(logics_perform_action(actions[0], play, params[0]) == 1);
	assert(logics_perform_action(actions[1], play, params[0]) == 0);
	assert(logics_perform_action(actions[2], play, params[12]) == 1);
	assert(logics_perform_action(actions[3], play, params[0]) == 1);
	assert(logics_perform_action(actions[4], play, params[18]) == 1);
	assert(logics_perform_action(actions[5], play, params[19]) == 0);
	assert(logics_perform_action(actions[6], play, params[19]) == 1);
	assert(logics_perform_action(actions[7], play, params[19]) == 1);

	assert(play -> board[4][2] == EMPTY);
	assert(play -> board[5][5] == 0);
	assert(play -> board[5][1] == EMPTY);
	assert(play -> board[1][5] == EMPTY);
	assert(play -> board[2][0] == EMPTY);
	assert(play -> board[7][5] == EMPTY);
	assert(play -> board[7][1] == EMPTY);
	assert(play -> board[3][5] == EMPTY);
	assert(play -> board[2][6] == 0);
	assert(play -> board[1][7] == 2);
	
	printf("Success!!!\n");
	return 0;
}
