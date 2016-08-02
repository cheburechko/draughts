#include "board.h"
#include "logics.h"
#include "moves.h"
#include "rules.h"
#include "interface.h"
#include "ai.h"
#include "dirent.h"
#include <string.h>
#include <stdlib.h>
#include "errors.h"

void print_history(gui_info_window * win, board_play * play)
{
	char * entry;
	while (win -> lines + 1 < play -> cur_move)
	{
		entry = gui_get_history_entry(play, ++win -> lines);
		win -> text = gui_add_string(win -> text, entry);
		free(entry);
	}
	win -> cur_line = min(max(0, win -> lines - win -> height), 
						  win -> cur_line + 1);
	gui_hide_window(win -> subwin);
	gui_draw_info_window(win, 0);
}

// check file type
int check_file_type(char * file)
{
	if (!file)
		return 0;

	int length = strlen(file);

	if (length < 5)
		return 0;
	
	return file[length - 1] == 't' && file[length - 2] == 'x' &&
		   file[length - 3] == 't' && file[length - 4] == '.';
}


void print_error_message(char * message, gui_info_window * win)
{
	// get an error message
	char ** errors = errors_get_separate();
	errors = gui_add_string(errors, message);
	gui_replace_info(win, errors);
	gui_call_info_window(win);
}
char * main_menu_choices[] =
{
	"New Game",
	"Choose Rules",
	"Read Rules",
	"Quit"
};

char * player_choices[] =
{
	"Human",
	"Easy Bot",
	"Medium Bot",
	"Hard Bot"
};

char * ask_black_player = "Choose black player.";
char * ask_white_player = "Choose white player.";

gui_menu_style style = 
{
	.name_color = GUI_BKGR,
};

int main(int argc, char ** argv)
{
	gui_init();
	// initializing
	rules_rules * rules = rules_get_default();
	board_play * play;
	moves_state * moves;
	ai_bot * bot0;
	ai_bot * bot1;

	gui_board * board;

	gui_info_window * help_window = gui_create_info_window
			(LINES - 2, COLS - 2, 1, 1, rules -> description, "Rules");
	gui_info_window * error_window = gui_create_info_window
			(LINES - 2, COLS - 2, 1, 1, 0, "ERRORS");
	gui_info_window * game_help_window = gui_create_info_window
			(LINES - 1, 20, 0, 0, "Arrow keys control cursor on the board. ENTER - select board field. Press 'q' to quit", "Help");
	gui_info_window * history_window = 0;
	gui_status_line * console_line = 
			gui_create_status_line(COLS, LINES - 1, 0);

	DIR * home_dir;
	struct dirent * entry;

	char ** rules_list = 0;
	int rules_num = 0, line = 1;


	gui_menu * main_menu = gui_create_menu(8, 20, "Draughts", 
							main_menu_choices, 0, 4, &style, 0);

	gui_menu * rules_menu = 0;

	gui_menu * choose_player_menu = gui_create_menu(14, 40, 
			ask_white_player, player_choices, 
			"Press SPACE to return to main menu\nPress q to quit",
			4, &style, "q ");

	// starting game process
	int input = 0;
	while (input != -'q')
	{
		input = gui_call_menu(main_menu);
		switch(input)
		{
			case 1: // new game
				// choosing players
				choose_player_menu -> name = ask_white_player;
				input = gui_call_menu(choose_player_menu);
				if (input == -'q' || input == - ' ')
					break;
				switch(input)
				{
					case 1: 
						input = 0; 
						break;
					default:
						input = rules -> ai_bots[input - 2];
						break;
				}
				bot0 = ai_create_bot(input, 0);

				choose_player_menu -> name = ask_black_player;
				input = gui_call_menu(choose_player_menu);
				if (input == -'q' || input == - ' ')
				{
					ai_destroy_bot(bot0);
					break;
				}
				switch(input)
				{
					case 1: 
						input = 0; 
						break;
					default:
						input = rules -> ai_bots[input - 2];
						break;
				}
				bot1 = ai_create_bot(input, 1);

				// initializing a play
				play = board_new_play(rules);
				board = gui_create_board(play, 2, 4);
				moves = moves_get(play);

				// preparing help window
				gui_draw_info_window(game_help_window, 0);
				history_window = gui_create_info_window
					(LINES - 1, 20, 0, COLS - 21, 0, "History");
				gui_draw_board(board);
				halfdelay(MAIN_DELAY);

				// start game cycle
				while (input != 'q')
				{
					if (play -> cur_move >= 500)
						break;
					input = getch();
					flushinp();
					// show history
					print_history(history_window, play);
					// leave if a game is over
					if (!moves)
						break;
					if (!moves[0].params)
						break;
					if (!(play -> cur_player))
					{
						gui_print_status_line(console_line, "White turn");
						if (bot0 -> depth > 0)
							moves = ai_make_a_move(bot0, play, moves);
						else
							moves = gui_board_make_a_move(board, moves);
					}
					else
					{
						gui_print_status_line(console_line, "Black turn");
						if (bot1 -> depth > 0)
							moves = ai_make_a_move(bot1, play, moves);
						else
							moves = gui_board_make_a_move(board, moves);
					}
					// redraw board
					gui_draw_board(board);
				}
				
				// reset typing mode
				nocbreak();
				cbreak();

				if (play -> cur_move >= 500)
					gui_print_status_line(console_line,
					"A DRAW. Type q to continue.");
				if (play -> cur_player)
					gui_print_status_line(console_line,
					"WHITE WON THE GAME!!! CONGRATS. Type q to continue.");
				else
					gui_print_status_line(console_line,
					"BLACK WON THE GAME!!! CONGRATS. Type q to continue.");

				while (getch() != 'q');

				gui_clear_status_line(console_line);
				// hide windows
				gui_destroy_info_window(history_window);
				gui_hide_window(game_help_window -> win);
				// free memory
				ai_destroy_bot(bot0);
				ai_destroy_bot(bot1);
				gui_destroy_board(board);
				board_destroy_play(play);
				moves_destroy_moves(moves);

				break;

			case 2: // choose rules
				rules_num = 0;

				// get list of txt files from the app dir
				home_dir = opendir(".");
				while (entry = readdir(home_dir))
					rules_num += check_file_type(entry -> d_name);
				rewinddir(home_dir);
				rules_list = (char **) malloc((rules_num + 1)
												* sizeof(char *));
				// establish default rules
				rules_num = 1;
				rules_list[0] = "Default (russian checkers)";

				while (entry = readdir(home_dir))
					if (check_file_type(entry -> d_name))
						rules_list[rules_num++] = entry -> d_name;
				
				// create a menu
				rules_menu = gui_create_menu(30, 60, "Rules", rules_list,
					"Press SPACE to go to previous menu\nPress q to quit\n",
					rules_num, &style, "q ");

				input = gui_call_menu(rules_menu);
				switch(input)
				{
					case -'q': case -' ':
						break;

					case 1:
						rules_destroy(rules);
						rules = rules_get_default();
						gui_replace_info(help_window, gui_break_string
								(rules -> description, help_window -> width));
						gui_print_status_line(console_line,
								"Rules were read susccessfully!!!");
						break;

					default:
						rules_destroy(rules);
						rules = rules_get(rules_list[input - 1]);
						if (!rules)
						{
							print_error_message
								("Rules are reset to default\n", error_window);
							rules = rules_get_default();
						}
						else
							gui_print_status_line(console_line,
											"Rules were read susccessfully!!!");
						gui_replace_info(help_window, gui_break_string
								(rules -> description, help_window -> width));
						break;

				}

				gui_destroy_menu(rules_menu);
				free(rules_list);
				closedir(home_dir);


				break;

			case 3:
				gui_call_info_window(help_window);
				break;

			case 4:
				input = - 'q';
				break;
		}
	}
	gui_destroy_menu(main_menu);
	gui_destroy_menu(choose_player_menu);

	gui_destroy_info_window(help_window);
	gui_destroy_info_window(game_help_window);
	gui_destroy_info_window(error_window);

	gui_destroy_status_line(console_line);

	gui_destroy();
	rules_destroy(rules);

}
