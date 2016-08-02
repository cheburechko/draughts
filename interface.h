#ifndef __draughts_interface_h
#define __draughts_interface_h

#include "board.h"
#include "moves.h"
#include <menu.h>

#define MAIN_DELAY 4
enum gui_color_pairs { GUI_BKGR = 32,
						GUI_B_ON_Y = 50, GUI_B_ON_R, GUI_W_ON_Y, 
						GUI_W_ON_R, GUI_HLIGHT_B, GUI_HLIGHT_W,
						GUI_CURSOR_B, GUI_CURSOR_W, GUI_G_ON_Y,
						GUI_G_ON_R, GUI_HLIGHT_G, GUI_CURSOR_G };

char * gui_error_messages[100];
extern int gui_error_messages_count;

void print(board_play * play);

typedef struct gui_menu_style
{
	chtype box_color, name_color, choices_color;
	char mark;
} gui_menu_style;

typedef struct gui_menu
{
	WINDOW * win, * subwin, * helpwin;
	MENU * menu;
	ITEM ** items;
	int lines,cols, help_lines;
	char * name, * keys, * help;
	gui_menu_style * style;
} gui_menu;

typedef struct gui_board
{
	WINDOW * win, * subwin;
	int sq_width, sq_height;
	board_play * play;
	int x,y,height,width;
	int cursor_x, cursor_y, hlight_x, hlight_y;
} gui_board;

typedef struct gui_info_window
{
	WINDOW * win, * subwin;

	char ** text, *header;
	int width, height;
	int lines, cur_line;
} gui_info_window;

typedef struct gui_status_line
{
	WINDOW * win;
	int cols;
} gui_status_line;

// init/destroy gui
void gui_init();

void gui_destroy();

// utility funcs

void print_in_middle(WINDOW * win, int starty, int startx,
					 int width, char * string, chtype color);

// clear window area on the screen
void gui_hide_window(WINDOW * win);

// break a string in lines according to given width of the window
char ** gui_break_string(char * message, int width);

// add a new string to the text. return new text
char ** gui_add_string(char ** text, char * string);

char * gui_cast_int_to_str(int a);

char * gui_get_history_entry(board_play * play, int i);

// working with menus
gui_menu * gui_create_menu(int lines, int cols, 
					char * name, char ** choices, char * help, int size,
					gui_menu_style * style, char * keys);

int gui_call_menu(gui_menu * menu);

void gui_destroy_menu(gui_menu * menu);

// working with boards
gui_board * gui_create_board(board_play * play, int sq_height, 
							 int sq_width);

void gui_draw_board(gui_board * board);

void gui_hide_board(gui_board * board);

void gui_destroy_board(gui_board * board);

moves_state * gui_board_make_a_move(gui_board * board, moves_state * moves);

// working with info windows
gui_info_window * gui_create_info_window(int lines, int cols, int y, int x,
										char * message, char * header);

char ** gui_break_string(char * message, int width);

void gui_replace_info(gui_info_window * win, char ** text);

void gui_destroy_info_window(gui_info_window * win);

void gui_draw_info_window(gui_info_window * win, int flag);

void gui_call_info_window(gui_info_window * win);

// woking with status line
gui_status_line * gui_create_status_line(int cols, int y, int x);

void gui_clear_status_line(gui_status_line * line);

void gui_print_status_line(gui_status_line * line, char * message);

void gui_destroy_status_line(gui_status_line * line);

#endif
