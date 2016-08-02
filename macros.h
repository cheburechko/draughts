#ifndef __draughts_macros_h
#define __draughts_macros_h

enum logicgs_conditions_descriptor { CON_END = 1, CON_START, CON_ENEMY,
	CON_DEFAULT, CON_EMPTY, CON_ALLY, CON_OBS, CON_COUNT, CON_PLAYER,
	CON_NO_MOVES, CON_PIECE, CON_COORD, CON_BOOL, CON_NUM, CON_TARGET,
	CON_ANY, CON_LINE, CON_LAST_PLAYER, CON_CUR_PLAYER, CON_COLUMN,
	CON_LR_DIAG, CON_RL_DIAG, CON_COUNTER, CON_OFFSET, CON_BYTE };

enum logics_operator_descriptors { OP_EQUAL = '=', OP_INEQUAL = '@',
	OP_AND = '&', OP_OR = '|' };

enum logics_action_descriptors { ACT_REPLACE = 1, ACT_ABORT, 
	ACT_REPLACE_OBS, ACT_REPEAT, ACT_NEXT, ACT_CONVERT, 
	ACT_INC_COUNTER, ACT_DEC_COUNTER, ACT_REMOVE };

enum default_macros { TEMP = 125, WALL, EMPTY };

#endif
