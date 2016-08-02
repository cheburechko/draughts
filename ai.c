#include "ai.h"
#include "moves.h"
#include "board.h"
#include "logics.h"
#include "rules.h"
#include "interface.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int ai_debug = 0;
int ai_processed_nodes = 0;
int ai_max_depth = 0;
int ai_buf_depth = 0;

const int infinity = 1000000;

ai_bot * ai_create_bot(int depth, int index)
{
	ai_bot * result = (ai_bot *) malloc(sizeof(ai_bot));
	result -> depth = depth;
	result -> index = index;
	return result;
}

void ai_destroy_bot(ai_bot * bot)
{
	free(bot);
}

// function that evaluates the currnt state of the play
int ai_evaluate(board_play * play, moves_state * moves)
{
	
	int result = 0;
	char s[100];
	if (!moves[0].params)
		if (play -> cur_player)
			result = infinity;
		else
			result = -infinity;
	else
	{
	int i;
	// count pieces weight
	if (play -> rules -> ai_eval_piece_weight)
		for (i = 0; i < play -> rules -> pieces; i++)
			result += play -> rules -> 
					ai_eval_piece_weight[i] * play -> piece_count[i];

	// count board position weight
	/*int x,y;
	if (play -> rules -> ai_eval_board)
		for (x = 0; x < play -> rules -> board_width; x++)
			for (y = 0; y < play -> rules -> board_height; y++)
				if (play -> board[x][y] != EMPTY)
					if (play -> rules -> piece_owners[play -> board[x][y]])
						result -= play -> rules -> ai_eval_board
								[play -> rules -> board_width - 1 - x]
								[play -> rules -> board_height - 1 - y];
					else
						result += play -> rules -> ai_eval_board[x][y];*/

	
	for (i = 0; i < play -> player_count[0]; i++)
		result += play -> rules -> ai_eval_board
				[play -> piece_list[0][i]][play -> piece_list[1][i]];
	for (i = 0; i < play -> player_count[1]; i++)
		result -= play -> rules -> ai_eval_board
			[play -> rules -> board_width - play -> piece_list[2][i] - 1]
			[play -> rules -> board_height - play -> piece_list[3][i] - 1];
	

	// TODO count conditions weight	
	}
	if (ai_debug)
	{
		print(play);
		printf("End of the tree\n");
		printf("%d %d \n", play -> cur_player, result);
		if (ai_debug == 1)
			gets(s);
	}

	return result;
}

// search for a move (alpha-beta)
int ai_search(board_play * play, moves_state * moves, int depth, 
				int a, int b)
{
	ai_processed_nodes++;
	// if the depth is over or the game is finished, return
	// the evaluation functions value
	if (!moves[0].params)
		return ai_evaluate(play, moves);

	int l = 0, player = play -> cur_player;
	char s[100];

	// loop vars
	moves_state * next_moves;
	int value = -infinity;
	if (player)
		value = infinity;

	int next_depth = depth - 1;

	// if the deal comes to fight, force search
	if (play -> rules -> moves[moves[0].params[5]].priority)
	{
		next_depth++;
		ai_buf_depth++;
		if (ai_buf_depth > ai_max_depth)
			ai_max_depth = ai_buf_depth;
	}

	if (next_depth <= 0)
		return ai_evaluate(play, moves);

	while (moves[l].params)
	{
		if (ai_debug)
		{
			print(play);
			printf("Depth %d\n", depth);
			printf("Current a:%d b:%d\nPlayer moving: %d", a, b, 
											play -> cur_player);
			if (ai_debug == 1)
				gets(s);
		}
		// make a move on a copy
		next_moves = moves_exec(play, &(moves[l]));
		// check value  of evaluation func
		if (player)
		{
			value = min(value, ai_search(play, next_moves, next_depth,
								 a, b));
			b = min(b, value);
		}
		else
		{
			value = max(value, ai_search(play, next_moves, next_depth,
								 a, b));
			a = max(a, value);
		}
		l++;
		// roll back
		moves_unmake(play);
		// free memory
		moves_destroy_moves(next_moves);
		if (a >= b)
			break;
	}

	if (play -> rules -> moves[moves[0].params[5]].priority)
		ai_buf_depth--;
	return value;
}

moves_state * ai_make_a_move(ai_bot * bot, board_play * play,
								moves_state * moves)
{
	// debugging info
	ai_processed_nodes = 0;
	ai_buf_depth = ai_max_depth = bot -> depth;

	int * possible_moves = (int *) malloc(sizeof(int) *
									play -> rules -> max_avail_moves);
	possible_moves[0] = 0;
	int i = 0, best_value, value, len = 1;
	moves_state * buf_moves;
	int a = -infinity, b = infinity, next_value;
	int depth = bot -> depth;
	char s[100];
	if (play -> rules -> moves[moves[0].params[5]].priority)
		depth++;

	if (!(moves[0].params && !moves[1].params))
		while (moves[i].params)
		{
			if (ai_debug)
			{
				print(play);
				printf("Depth %d\n", bot -> depth);
				printf("Current a:%d b:%d\nPlayer moving: %d", a, b, 
												play -> cur_player);
				if (ai_debug == 1)
					gets(s);
			}
			// copy play and make a move on it
			buf_moves = moves_exec(play, &(moves[i]));
	
			// get value
			next_value = ai_search(play, buf_moves, depth, a, b);
			if (bot -> index)
			{
				if (next_value < b)
				{
					b = next_value;
					len = 1;
					possible_moves[0] = i;
				}
				else if (next_value == b)
				{
					possible_moves[len] = i;
					len++;
				}	
			}
			else
			{
				if (next_value > a)
				{
					a = next_value;
					len = 1;
					possible_moves[0] = i;
				}
				else if (next_value == a)
				{
					possible_moves[len] = i;
					len++;
				}	
			}

			moves_unmake(play);
			moves_destroy_moves(buf_moves);
			/*if (b == -infinity && bot -> index || 
			a == infinity && !(bot -> index))
			break;*/
			i++;
		}

	moves_state * result = moves_exec(play, 
				&(moves[possible_moves[random() % len]]));
	moves_destroy_moves(moves);
	free(possible_moves);
	return result;
}
