#include "interface.h"

int main ()
{
	gui_init();
	char * opts[] =
	{
		"Opt1",
		"Opt2",
		"Opt3",
		"Opt4",
		"Opt4",
		"Opt4",
		"Opt4",
		"Opt4",
		"Opt4",
		"Opt4",
	};

	int i, c;
	char s[] = "1 22 333 4444 55555 666666 7777777 88888888  abcs\nasfa\nasdfd\nasf\ndsfqsdd\nasdfasddfa\nasdfadf\nadsfadsfa sdfasdf  adsfas df asdfa sdf ";
	for (i = 0; i < LINES; i++) 
		mvhline(i, 0, '#', COLS);
	refresh();
	cbreak();

	gui_info_window * info = gui_create_info_window(10, 20, 15, 40, s, "HEADER");
	gui_call_info_window(info);
	gui_destroy_info_window(info);

	init_pair(1, COLOR_RED, COLOR_BLACK);
	gui_menu_style style = {
		.name_color = COLOR_PAIR(1)
	};
	gui_menu * menu1 = gui_create_menu(10, 30, "MENU1", opts, 0, 10,
									&style, 0);
	gui_menu * menu2 = gui_create_menu(8, 15, "MENU2", opts, "q,w,SPACE - quit",
									   10, &style, "q w");
	gui_menu * menu3 = gui_create_menu(12, 20, "MENU3", opts, "q - quit\nSPACE - quit",
									   10, &style, "q ");

	mvprintw(LINES - 1, 0, "Pairs:%d Colors:%d ", COLOR_PAIRS, COLORS);


	for (i = 0; i < COLORS; i++)
	{
		init_pair(i+2, i, i);
		attron(COLOR_PAIR(i+2));
		printw("   ");
	}

	while (getch() != 'x');
	int res = 1;
	while (res != -2)
	{
		switch(res)
		{
			case 1:
				res = gui_call_menu(menu1);
				break;
			case 2:
				res = gui_call_menu(menu2);
				break;
			case 3:
				res = gui_call_menu(menu3);
				break;
			default:
				res = -2;
				break;
		}
		mvprintw(1, 1, "The result is %d", res);
	}

	rules_rules * rules = rules_get_default();
	board_play * play = board_new_play(rules);

	gui_board * board = gui_create_board(play, 2, 4);
	gui_draw_board(board);
	while (getch() != 'x');
	gui_board_hide(board);
	while (getch() != 'x');
	gui_destroy_menu(menu1);
	gui_destroy_menu(menu2);
	gui_destroy_menu(menu3);
	gui_destroy_board(board);
	gui_destroy();
	board_destroy_play(play);
	rules_destroy(rules);
	return;
}
