#include "board.h"
#include "moves.h"
#include "macros.h"
#include "interface.h"
#include <stdlib.h>

// creates a board representation connected with play
gui_board * gui_create_board(board_play * play, int sq_height, int sq_width)
{
	gui_board * result = (gui_board *) malloc(sizeof(gui_board));
	int width = sq_width * play -> rules -> board_width + 6;
	int height = sq_height * play -> rules -> board_height + 4;
	int x = (COLS - width) / 2, y = (LINES - height) / 2;
	WINDOW * win = newwin(height, width, y, x);

	result -> win = win;
	result -> height = height;
	result -> width = width;
	result -> x = x;
	result -> y = y;
	result -> cursor_x = -1;
	result -> cursor_y = -1;
	result -> hlight_x = -1;
	result -> hlight_y = -1;
	result -> play = play;
	result -> sq_width = sq_width;
	result -> sq_height = sq_height;
}

// redraw a board
void gui_draw_board(gui_board * board)
{
	int x, y, xx, yy;
	box(board -> win, 0, 0);
	int ops = 0, color = 0;
	rules_rules * rules = board -> play -> rules;
	for (y = rules -> board_height - 1; y >= 0; y--)
	{
		// add number of line
		wattrset(board -> win, GUI_BKGR);
		mvwaddch(board -> win, (rules -> board_height - y) *
				 board -> sq_height, 2, (y + 1) % 10 + '0');
		if ((y + 1) / 10)
			mvwaddch(board -> win, (rules -> board_height - y) *
					 board -> sq_height, 1, (y + 1) / 10 + '0');

		for (x = 0; x < rules -> board_width; x++)
		{
			// set square color
			char c = board -> play -> board[x][y];

			if (c != EMPTY)
				color = rules_get_player(board -> play -> rules, c);

			if (x == board -> cursor_x && y == board -> cursor_y)
				if (color == 2)
					wattrset(board -> win, COLOR_PAIR(GUI_CURSOR_G));
				else if (color)
					wattrset(board -> win, COLOR_PAIR(GUI_CURSOR_B));
				else
					wattrset(board -> win, COLOR_PAIR(GUI_CURSOR_W));
			else if (x == board -> hlight_x && y == board -> hlight_y)
				if (color == 2)
					wattrset(board -> win, COLOR_PAIR(GUI_HLIGHT_G));
				else if (color)
					wattrset(board -> win, COLOR_PAIR(GUI_HLIGHT_B));
				else
					wattrset(board -> win, COLOR_PAIR(GUI_HLIGHT_W));
			else if ((x+y) % 2 == rules -> board_height % 2)
				if (color == 2)
					wattrset(board -> win, COLOR_PAIR(GUI_G_ON_R));
				else if (!color)
					wattrset(board -> win, COLOR_PAIR(GUI_W_ON_R));
				else
					wattrset(board -> win, COLOR_PAIR(GUI_B_ON_R));
			else
				if (color == 2)
					wattrset(board -> win, COLOR_PAIR(GUI_G_ON_Y));
				else if (!color)
					wattrset(board -> win, COLOR_PAIR(GUI_W_ON_Y));
				else
					wattrset(board -> win, COLOR_PAIR(GUI_B_ON_Y));


			// deifne square icon
			//printw("%d ",c);
			if (c == EMPTY) 
				c = ' ';
			else if (c == TEMP || c == WALL)
				c= 'X';
			else
				c = rules -> piece_icon[board -> play -> board[x][y]];

			// print square
			for (yy = 0; yy < board -> sq_height; yy++)
				mvwhline(board -> win, (rules -> board_height - y - 1)
				* board -> sq_height + yy + 1,
				 5 + x * board -> sq_width, c, board -> sq_width);

		}
	}

	wattrset(board -> win, GUI_BKGR);
	for (x = 0; x < rules -> board_width; x++)
		// write col number
		mvwaddch(board -> win, 
				 board -> sq_height * rules -> board_height + 1,
				 x * board -> sq_width + 5 + (board -> sq_width - 1) / 2,
				 x + 'a');
				 
	wrefresh(board -> win);

}

// give control of the board to player, get his move, execute it and
// get next moves

moves_state * gui_board_make_a_move(gui_board * board, moves_state * moves)
{
	moves_state * result = moves;
	int start_x, start_y, end_x, end_y, c;
	char input[10], player;
	rules_rules * rules = board -> play -> rules;

	if (board -> cursor_x == -1)
	{
		board -> cursor_x = 0;
		board -> cursor_y = 0;
	}

	keypad(board -> win, TRUE);
	nocbreak();
	cbreak();

	while (result == moves)
	{
		start_x = -1;
		end_x = -1;
		gui_draw_board(board);
		while (end_x == -1)
		{
			c = getch();
			switch (c)
			{
				case KEY_UP:
					if (board -> cursor_y < rules -> board_height - 1)
						board -> cursor_y++;
					break;

				case KEY_DOWN:
					if (board -> cursor_y > 0)
						board -> cursor_y--;
					break;

				case KEY_LEFT:
					if (board -> cursor_x > 0)
						board -> cursor_x--;
					break;

				case KEY_RIGHT:
					if (board -> cursor_x < rules -> board_width - 1)
						board -> cursor_x++;
					break;

				case 10:
					player = rules_get_player(board -> play -> rules,
						board -> play -> board[board -> cursor_x]
											  [board -> cursor_y]);
					if (player == board -> play -> cur_player)
					{
						start_x = board -> cursor_x;
						start_y = board -> cursor_y;
						board -> hlight_x = start_x;
						board -> hlight_y = start_y;
						break;
					}
					if (start_x != -1)
					{
						end_x = board -> cursor_x;
						end_y = board -> cursor_y;
						board -> hlight_x = -1;
						board -> hlight_y = -1;
					}
					break;

				case 'q':
					halfdelay(MAIN_DELAY);
					return 0;
					break;
			}
			gui_draw_board(board);
		}
		input[0] = 'a' + start_x;
		input[2] = 'a' + start_y;
		input[4] = 'a' + end_x;
		input[6] = 'a' + end_y;
		input[1] = input[3] = input[5] = ' ';
		input[7] = 0;
		result = moves_define_and_make(board -> play, input, moves);
	}
	halfdelay(MAIN_DELAY);
	return result;
}


void gui_board_hide(gui_board * board)
{
	int x, y;
	wattrset(board -> win, GUI_BKGR);
	for (x = 0; x < board -> width; x++)
		mvwvline(board -> win, 0, x, ' ', board -> height);
	wrefresh(board -> win);
}


void gui_destroy_board(gui_board * board)
{
	gui_board_hide(board);
	delwin(board -> win);
	free(board);
}

