#include "board.h"
#include "logics.h"
#include "moves.h"
#include "rules.h"
#include "interface.h"
#include "ai.h"
#include "errors.h"
#include <stdlib.h>
#include <stdio.h>

moves_state * player_make_a_move(board_play * play, 
									moves_state * moves)
{
	moves_state * result = moves;
	char * s;
	while (result == moves)
	{
		printf("%d player:", play -> cur_player);
		gets(s);
		result = moves_define_and_make(play, s, moves);
	}

	return result;
}


int main(int argc, char ** argv)
{
	ai_bot * bot1, * bot2;	
	rules_rules * rules = 0;

	if (argc >= 2)
		rules = rules_get(argv[1]);

	if (argc >= 5)
	{
		ai_debug = argv[4][0] - '0';
		int d = 0, i = 0;
		while (argv[2][i])
			d = d * 10 + argv[2][i++] - '0';
		bot1 = ai_create_bot(d, 0);
		d = 0; i = 0;
		while (argv[3][i])
			d = d * 10 + argv[3][i++] - '0';
		bot2 = ai_create_bot(d, 1);
	}

	if (!rules)
	{
		char * err = errors_get();
		printf("%s", err);
		free(err);
		printf("No rules read, using default rules (russian shashki)\n");
		rules = rules_get_default();
	}
	else
		printf("Rules were successfully read\n");

	board_play * play = board_new_play(rules);
	moves_state * moves = moves_get(play);
	moves_state * new_moves;
	char s[100] = "";
	srandom(time(0));
	print(play);
	int l = 0, i = 0;

	while (s[0] != 'x' && moves[0].params)
	{
		printf("Press enter\n");
		if (ai_debug != 2)
			gets(s);
		
		if (argc <= 2)
		{
			if (s[0] == 'u')
			{
				moves_unmake(play);
				new_moves = moves_get(play);
			}
			else
			{
				l = 0;
				while(moves[l].params)
					l++;
				if (l == 0)
					break;
				i = random() % l;
				new_moves = moves_exec(play, &(moves[i]));
			}
			moves_destroy_moves(moves);
			moves = new_moves;
		}
		else 
		{
			if (!(play -> cur_player))
				if (bot1 -> depth)
				{
					printf("Bot1\n");
					moves = ai_make_a_move(bot1, play, moves);
				}
				else
					moves = player_make_a_move(play, moves);
			else
				if (bot2 -> depth)
				{
					printf("Bot2\n");
					moves = ai_make_a_move(bot2, play, moves);
				}
				else
					moves = player_make_a_move(play, moves);
		}

		print(play);
	}

	printf("The winner is %d\n", (play -> cur_player + 1) % 2);
	board_destroy_play(play);
	moves_destroy_moves(moves);
	if (argc >= 4)
	{
		ai_destroy_bot(bot1);
		ai_destroy_bot(bot2);
	}
	rules_destroy(rules);
}
