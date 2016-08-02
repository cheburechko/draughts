#include "interface.h"
#include <stdlib.h>
#include <string.h>

void gui_replace_info(gui_info_window * win, char ** text)
{
	if (!text)
		return;
	int i = 0;
	if (win -> text)
	{
		while (win -> text[i])
			free(win -> text[i++]);
		free(win -> text);
	}
	win -> text = text;
	i = -1;
	while(win -> text[++i]);
	win -> lines = i;
	win -> cur_line = 0;
}

gui_info_window * gui_create_info_window(int lines, int cols, int y, int x,
										 char * message, char * header)
{
	// connects a message with window
	gui_info_window * result = (gui_info_window *) 
								malloc(sizeof(gui_info_window));
	result -> win = newwin(lines, cols, y, x);
	result -> subwin = derwin(result -> win, lines - 2, cols - 2, 1, 1);
	result -> height = lines - 2;
	result -> width = cols - 2;
	result -> text = 0;
	result -> lines = 0;
	
	gui_replace_info(result, gui_break_string(message, result -> width));

	result -> cur_line = 0;
	result -> header = (char *) 
			malloc((strlen(header) + 1) * sizeof(char));
	strcpy(result -> header, header);
	return result;
}

void gui_destroy_info_window(gui_info_window * win)
{
	gui_hide_window(win -> win);
	int i;
	if (win -> text)
		for (i = 0; i <= win -> lines; i++)
			free(win -> text[i]);
	free(win -> text);
	free(win -> header);
	delwin(win -> subwin);
	delwin(win -> win);
	free(win);
}

void gui_draw_info_window(gui_info_window * win, int flag)
{
	wmove(win -> subwin, 0, 0);
	int i;
	if (win -> text)
		for (i = win -> cur_line; 
			 win -> text[i] && i < win -> cur_line + win -> height;
			 i++)
			wprintw(win -> subwin, win -> text[i]);
	box(win -> win, 0, 0);
	mvwprintw(win -> win, 0, 3, win -> header);
	if (flag)
		mvwprintw(win -> win, win -> height + 1, 3,
			  "q to quit, up and down to scroll");
	wrefresh(win -> win);
}

// show info window, control it and close it
void gui_call_info_window(gui_info_window * win)
{
	gui_draw_info_window(win, 1);
	int c = 0;
	while(c != 'q')
	{
		c = getch();
		switch(c)
		{
			case 'q':
				break;
			case KEY_UP:
				win -> cur_line = max(0, win -> cur_line - 1);
				//mvprintw(LINES - 1, 0, "%d %d %d  ", win -> cur_line, win -> lines, win -> height);
				gui_hide_window(win -> subwin);
				gui_draw_info_window(win, 1);
				break;
			case KEY_DOWN:
				win -> cur_line = min(max(0, win -> lines - win -> height),
									  win -> cur_line + 1);
				//`mvprintw(LINES - 1, 0, "%d %d %d  ", win -> cur_line, win -> lines, win -> height);
				gui_hide_window(win -> subwin);
				gui_draw_info_window(win, 1);
				break;
		}
		refresh();
	}
	gui_hide_window(win -> win);
}
