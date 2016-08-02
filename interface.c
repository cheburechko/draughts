#include <stdio.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"
#include "board.h"
#include "rules.h"
#include "parser.h"
#include "macros.h"
#include "moves.h"

int gui_error_messages_count = 0;

void print(board_play * play)
{
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
	int x = 0, y = 0;
	
	// upper border
	printf("   ");
	for (x = 0; x < play -> rules -> board_width; x++)
		printf("==");
	printf("=\n");

	// print board
	for (y = play -> rules -> board_height - 1; y >= 0; y--)
	{
		// number of line
		printf(" %d ", y + 1);

		// print pieces
		for (x = 0; x < play -> rules -> board_width; x++)
			if (play -> board[x][y] == EMPTY)
				printf("| ");
			else if (play -> board[x][y] == WALL ||
					 play -> board[x][y] == TEMP)
				printf("|X");
			else
				printf("|%c", play -> rules -> 
						piece_icon[play -> board[x][y]]);
		printf("|\n   ");

		// lower line
		for (x = 0; x < play -> rules -> board_width; x++)
			printf("==");
		printf("=\n");

	}

	// letters
	printf("   ");
	for (x = 0; x < play -> rules -> board_width; x++)
		printf(" %c", x + 'a');
	printf("\n");
}

void gui_init(void)
{
	initscr();
	start_color();
	init_pair(GUI_BKGR, COLOR_WHITE, COLOR_BLACK);
	init_pair(GUI_B_ON_Y, COLOR_BLACK, COLOR_YELLOW);
	init_pair(GUI_B_ON_R, COLOR_BLACK, COLOR_RED);
	init_pair(GUI_W_ON_Y, COLOR_WHITE, COLOR_YELLOW);
	init_pair(GUI_W_ON_R, COLOR_WHITE, COLOR_RED);
	init_pair(GUI_HLIGHT_B, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(GUI_HLIGHT_W, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(GUI_CURSOR_B, COLOR_BLACK, COLOR_CYAN);
	init_pair(GUI_CURSOR_W, COLOR_WHITE, COLOR_CYAN);
	init_pair(GUI_G_ON_Y, COLOR_GREEN, COLOR_YELLOW);
	init_pair(GUI_G_ON_R, COLOR_GREEN, COLOR_RED);
	init_pair(GUI_HLIGHT_G, COLOR_GREEN, COLOR_MAGENTA);
	init_pair(GUI_CURSOR_G, COLOR_GREEN, COLOR_CYAN);
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
}

void gui_destroy(void)
{
	endwin();
}

void print_in_middle(WINDOW * win, int starty, int startx,
					 int width, char * string, chtype color)
{
	int x,y;
	if (win == NULL)
		win = stdscr;
	getyx(win, y, x);

	if (width == 0)
		width = 80;

	y = starty;
	
	int length = strlen(string);
	x = startx + (width - length + 0.5) / 2;
	wattron(win, color);
	mvwprintw(win, y, x, string);
	wattroff(win, color);
	refresh();
}


void gui_hide_window(WINDOW * win)
{
	int x,y,maxx,maxy;
	getbegyx(win, y, x);
	getmaxyx(win, maxy, maxx);
	int height = maxy;
	int width = maxx;
	//mvprintw(LINES - 2, 0, "%d %d %d %d %d %d", x, y, maxx, maxy, width,
	//		 height);
	for (y = 0; y < height; y++)
		mvwhline(win, y, 0, ' ', width);
	wrefresh(win);
}

// break a string in lines according to given width of the window
char ** gui_break_string(char * message, int width)
{
	if (!message)
		return 0;
	int length = strlen(message), lastl = 1, l = 0, pos = 0, lines = 1;
	int size = 100;
	int * delimeters = (int *) malloc(size * sizeof(int));
	delimeters[0] = 0;
	// divide message in separate strings shorter than window width
	while (l < length)
	{
		while (!isspace(message[l]) && message[l])
			l++;
		pos += l - lastl;

		if (message[l] == '\t')
			pos += 8;
		else
			pos++;

		if (pos >= width || message[lastl - 1] == '\n')
		{
			delimeters[lines++] = lastl;
			pos = l - lastl + 1;
		}
		l++;
		if (lines >= size)
		{
			size *= 2;
			delimeters = realloc(delimeters, size);
		}
		lastl = l;
	}
	delimeters[lines] = length;

	char ** result = (char **) malloc((1 + lines) * sizeof(char *));
	for (l = 0; l < lines; l++)
	{
		result[l] = malloc
				((delimeters[l + 1] - delimeters[l] + 3) * sizeof(char));
		strncpy(result[l], message + sizeof(char) * delimeters[l],
				delimeters[l + 1] - delimeters[l]);
		if (result[l][delimeters[l + 1] - delimeters[l] - 1] != 
			'\n')
		{
			result[l][delimeters[l + 1] - delimeters[l]] = '\n';
			result[l][delimeters[l + 1] - delimeters[l] + 1] = 0;
		}
		else
			result[l][delimeters[l + 1] - delimeters[l]] = 0;
		
	}
	result[lines] = 0;
	free(delimeters);
	return result;
}

// add a new string to the text. return new text
char ** gui_add_string(char ** text, char * string)
{
	int i = 0;
	if (text)
		while (text[i++]);
	else
		i = 1;
	char ** result = realloc(text, (i + 1) * sizeof(char *));
	result[i] = 0;
	result[i - 1] = (char * ) malloc((strlen(string) + 1) * sizeof(char));
	strcpy(result[i - 1], string);
	return result;
}

char * gui_cast_int_to_str(int a)
{
	char * s = (char *) malloc(20 * sizeof(char));
	if (a == 0)
	{
		s[0] = '0';
		s[1] = 0;	
		return s;
	}
	int l = 0;
	while (a > 0)
	{
		s[l++] = a % 10 + '0';
		a /= 10;
	}
	int i = l; char c;
	while (i > (l + 1)/2)
	{
		c = s[l - i];
		s[l - i] = s[i - 1];
		s[--i] = c;
	}
	s[l] = 0;
	return s;
}

char * gui_get_history_entry(board_play * play, int i)
{
	int l = 0;
	char * entry = (char *) malloc(20 * sizeof(char));
	char * buf;
	entry[0] = 0;

	buf = gui_cast_int_to_str(i);
	strcat(entry, buf);
	free(buf);

	strcat(entry, ": ");
	while(entry[l++]);
	entry[l - 1] = play -> history[i].params[0] + 'a';
	entry[l] = 0;

	buf = gui_cast_int_to_str(play -> history[i].params[1] + 1);
	strcat(entry, buf);
	free(buf);

	strcat(entry, " -> ");
	while(entry[l++]);
	entry[l - 1] = play -> history[i].params[2] + 'a';
	entry[l] = 0;

	buf = gui_cast_int_to_str(play -> history[i].params[3] + 1);
	strcat(entry, buf);
	free(buf);

	while(entry[l++]);
	entry[l] = 0;
	entry[l - 1] = '\n';

	return entry;
}
