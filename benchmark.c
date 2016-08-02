#include "ai.h"
#include <time.h>
#include <stdio.h>
#include "rules.h"
#include "board.h"
#include "moves.h"

int main()
{
	clock_t time1 = clock();
	rules_rules * rules = rules_get_default();
	clock_t time2 = clock();
	printf("Creating rules: %d\n", (int) (time2-time1));

	time1 = clock();
	board_play * play = board_new_play(rules);
	time2 = clock();
	printf("Creating play: %d\n", (int) (time2-time1));

	time1 = clock();
	moves_state * moves = moves_get(play);
	time2 = clock();
	printf("Getting moves: %d\n", (int) (time2-time1));

	ai_bot * bot = ai_create_bot(8, 0);

	time1 = clock();
	moves = ai_make_a_move(bot, play, moves);
	time2 = clock();
	printf("AI: %d\n", (int) (time2-time1));
	printf("Nodes %d\n", ai_processed_nodes);
	printf("Max_depth %d\n", ai_max_depth);

	int i;
	time1 = clock();
	for (i = 0; i < 500000; i++)
		moves_destroy_moves(moves_get(play));
	time2 = clock();
	printf("Getting and destroying moves: %d\n", (int) (time2-time1));

	time1 = clock();
	for (i = 0; i < 500000; i++)
	{
		moves_destroy_moves(moves_exec(play, moves));
		moves_unmake(play);
	}
	time2 = clock();
	printf("Making/unmaking moves %d\n", (int) (time2-time1));

	time1 = clock();
	for (i = 0; i < 500000; i++)
		ai_evaluate(play, moves);
	time2 = clock();
	printf("Evaluating: %d\n", (int) (time2-time1));

	time1 = clock();
	for (i = 0; i< 500000; i++)
		moves_check_events(play);
	time2 = clock();
	printf("Checking events: %d\n", (int) (time2-time1));

	moves_destroy_moves(moves);
	board_destroy_play(play);
	rules_destroy(rules);
	
}
