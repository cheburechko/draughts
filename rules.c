#include "rules.h"
#include "macros.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include "errors.h"



//=========================Evil global variables============================


// Gloabal parameters

// Board measurements
static const int default_board_width = 8, default_board_height = 8;

/* board
 *		 01234567
 *		 =========> Y, board_height
 *		0II# # # #
 * 		1II # # #
 *		2II# # #
 * 		3II # # #
 *		4\/ 
 *		  X, board_width
 */

static const char default_sample[8][9] = {
	{0, EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 0},
	{EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 2, 0},
	{0, EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 0},
	{EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 2, 0},
	{0, EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 0},
	{EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 2 ,0},
	{0, EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY ,0},
	{EMPTY, 0, EMPTY, EMPTY, EMPTY, 2, EMPTY, 2 ,0},
};


/*  Pieces specifications:
 *		0 - white man
 *		1 - white king
 *		2 - black man
 *		3 - black king
 */

static const int default_pieces = 4;
static const int default_piece_owners[] = {0, 0, 1, 1};
static const char default_piece_icon[] = {'o', '@', '*', '$'};


// events
static const int default_event_count = 3;

static const rules_event default_events[] = 
{
	{
	 .con = {CON_DEFAULT, 0},
	 .act = {ACT_CONVERT, CON_LINE, CON_NUM, '7', CON_PIECE, CON_NUM, '0', 
	 		CON_PIECE, CON_NUM, '1', 0}
	},
	{
	 .con = {CON_DEFAULT, 0},
	 .act = {ACT_CONVERT, CON_LINE, CON_NUM, '0', CON_PIECE, CON_NUM, '2',
	 		CON_PIECE, CON_NUM, '3', 0}
	},
	{
	 .con = {CON_CUR_PLAYER, '@', CON_LAST_PLAYER, 0},
	 .act = {ACT_REMOVE, CON_PIECE, CON_BYTE, TEMP, 0}
	}
};

static const int default_max_avail_moves = 60;
static const int default_moves_count = 5;

// Moves 
static const moves_move default_moves[5] = {
	//white man move
	{.type = 'm', .dx = "i1", .dy = "1", .priority = 0, .repeatable = 0,
	 .conditions = {
	 	{CON_PLAYER, CON_END, '=', CON_EMPTY, 0},
		{CON_DEFAULT, 0}
	 },
	 .actions = {
	 	{ACT_REPLACE, ACT_NEXT, 0},
		{ACT_ABORT, 0}
	 }
	},
	// black man move
	{.type = 'm', .dx = "i1", .dy = "1-", .priority = 0, .repeatable = 0,
	 .conditions = {
	 	{CON_PLAYER, CON_END, '=', CON_EMPTY, 0},
		{CON_DEFAULT, 0}
	 },
	 .actions = {
	 	{ACT_REPLACE, ACT_NEXT, 0},
		{ACT_ABORT, 0}
	 }
	},
	// man jump
	{.type = 'j', .dx = "i1", .dy = "i1", .priority = 1, .repeatable = 1,
	 .conditions = {
		{CON_PLAYER, CON_END, '@', CON_EMPTY, 0},
		{CON_PLAYER, CON_TARGET, '=', CON_ENEMY, 0},
		{CON_DEFAULT, 0}
	 },
	 .actions = {
	 	{ACT_ABORT, 0},
		{ACT_REPLACE_OBS, CON_BYTE, TEMP, ACT_REPLACE, ACT_REPEAT, 0},
		{ACT_ABORT, 0}
	 }
	},
	 // king move
	{.type = 'm', .dx = "ia", .dy = "ia", .priority = 0, .repeatable = 0,
	 .conditions = {
	  	{CON_PLAYER, CON_END, '@', CON_EMPTY, 0},
		{CON_OBS, CON_ANY, '=', CON_BYTE, 0, 0},
		{CON_DEFAULT, 0}
	 },
	 .actions = {
	  	{ACT_ABORT, 0},
		{ACT_REPLACE, ACT_NEXT, 0},
		{ACT_ABORT, 0}
	 }
	},
	 // king jump
	{.type = 'm', .dx = "ia", .dy = "ia", .priority = 1, .repeatable = 1,
	  .conditions = {
	  	{CON_PLAYER, CON_END, '@', CON_EMPTY, 0},
		{CON_OBS, CON_ALLY, '@', CON_BYTE, 0, 0},
		{CON_OBS, CON_ENEMY, '=', CON_BYTE, 1, 0},
		{CON_DEFAULT, 0}
	 },
	 .actions = {
	  	{ACT_ABORT, 0},
		{ACT_ABORT, 0},
		{ACT_REPLACE_OBS, CON_BYTE, TEMP, ACT_REPLACE, ACT_REPEAT, 0},
		{ACT_ABORT, 0}
	 }, 
	}
};

// An array of moves available for pieces
static const int default_pieces_moves[4][3] = {
	{2, 0, EMPTY},
	{4, 3, EMPTY},
	{2, 1, EMPTY},
	{4, 3, EMPTY}
};

// A flag that defines whether the longest possible move
// must be chosen
static const int default_longest_move = 0;

// pieces weight
static const int default_ai_eval_piece_weight[5] = {50, 250, -50, -250};

// position weight
static const int default_ai_eval_board[8][8] = 
{
	{1, 0, 2, 0, 5, 0, 8, 0},
	{0, 2, 0, 5, 0, 8, 0, 10},
	{1, 0, 3, 0, 7, 0, 9, 0},
	{0, 2, 0, 5, 0, 8, 0, 10},
	{1, 0, 3, 0, 7, 0, 9, 0},
	{0, 2, 0, 5, 0, 8, 0, 10},
	{1, 0, 3, 0, 7, 0, 9, 0},
	{0, 1, 0, 3, 0, 7, 0, 10}
};

// bots depths
static const int default_ai_bots[3] = {4, 6, 7};

static char default_description[] = "Rules\nAs in all draughts variants, Russian draughts is played by two people, on opposite sides of a playing board, alternating moves. One player has dark pieces, and the other has light pieces. Pieces move diagonally and pieces of the opponent are captured by jumping over them.\n\nThe rules of this variant of draughts are:\n\nBoard: The board is an 8×8 grid, with alternating dark and light squares. The left down square field should be dark.\n\nPieces: Traditionally, the colors of the pieces are black and white. There are two kinds of pieces: \"men\"' and \"kings\". Kings are differentiated as consisting of two normal pieces of the same color, stacked one on top of the other.\n\nStarting position: Each player starts with 12 pieces on the three rows closest to their own side. The row closest to each player is called the \"crownhead\" or \"kings row\". The white (lighter color) side moves first.\n\nHow to move. There are two ways to move a piece: simply sliding a piece diagonally forward to an adjacent and unoccupied dark square, or \"jumping\" one of the opponent's pieces. In this case, one piece would \"jump over\" the other, provided there is a vacant square on the opposite side for it to land on. Jumping can be done forward and backward. A piece that is jumped is captured and removed from the board. Multiple-jump moves are possible if, when the jumping piece lands, there is another piece that can be jumped. Jumping is mandatory and cannot be passed up to make a non-jumping move, nor can fewer than the maximum jumps possible be taken in a multiple-jump move. When there is more than one way for a player to jump, one may choose which sequence to make, not necessarily the sequence that will result in the most amount of captures. However, one must make all the captures in that sequence.\n\nKings: If a player's piece moves into the kings row on the opposing player's side of the board, that piece is said to be \"crowned\", becoming a \"king\" and gaining the ability to move freely multiple steps in any direction and jump over and hence capture an opponent piece some distance away and choose where to stop afterwards. If an ordinary piece moves into the kings row from a jump and it can continue to jump backwards as a king then the move should be continued. Also if there is a piece in the king's path that can be captured, the king must capture it, provided it's the player's move. There is no choice whether to do it or not, but the player can choose where to land after the capture.\n\nHow the game ends: A player wins by leaving the opposing player with no legal moves, i.e. by capturing or blocking all of the opposing player's pieces.";

//============================ Static functions ===========================



static void board_set_default(rules_rules * rules)
{
	int i,j,player;
	// board measurements
	rules -> board_width = default_board_width;
	rules -> board_height = default_board_height;

	// pieces info
	rules -> pieces = default_pieces;
	rules -> piece_owners = (char *) calloc(rules -> pieces, sizeof(char));
	rules -> piece_icon = (char *) malloc(sizeof(char) * rules -> pieces);
	rules -> piece_count = (int *) calloc(rules -> pieces, sizeof(int));

	for (i = 0; i < rules -> pieces; i++)
	{
		rules -> piece_owners[i] = default_piece_owners[i];
		rules -> piece_icon[i] = default_piece_icon[i];
		rules -> piece_count[i] = 0;
	}

	for (i = 0; i < 3; i++)
	{
		rules -> player_count[i] = 0;
		rules -> default_piece_list[i * 2] = (int *) calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));
		rules -> default_piece_list[i * 2 + 1] = (int *) calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));
	}

	// copy sample board
	rules -> sample = (char **) malloc(
								sizeof(char  *) * rules -> board_width);
	rules -> default_index_board = (int **) malloc(
								rules -> board_width * sizeof(int *));

	for (i = 0; i < rules -> board_width; i++)
	{
		rules -> sample[i] = (char *) malloc(
								sizeof(char) * (rules -> board_height));
		rules -> default_index_board[i] = (int *) calloc(
								rules -> board_height, sizeof(int));

		for (j = 0; j < rules -> board_height; j++)
		{
			rules -> sample[i][j] = default_sample[i][j];
			player = rules_get_player(rules, rules -> sample[i][j]);
			if (player != EMPTY)
			{
				// count pieces
				if (player != 2)
					rules -> piece_count[rules -> sample[i][j]]++;

				// save position of each piece
				rules -> default_piece_list[player * 2]
						[rules -> player_count[player]] = i;
				rules -> default_piece_list[player * 2 + 1]
						[rules -> player_count[player]] = j;
				rules -> default_index_board[i][j]
						= rules -> player_count[player]++;
			}
		}
	}
}

// Set default constants
static void moves_set_default(rules_rules * rules)
{
	int i,j;
	rules -> moves_count = default_moves_count;
	rules -> moves = (moves_move *) malloc(sizeof(moves_move) * 
										   rules -> moves_count);
	for (i = 0; i < rules -> moves_count; i++)
		rules -> moves[i] = default_moves[i];

	rules -> pieces_moves = (int **) malloc(sizeof(int *)*rules -> pieces);
	for (i = 0; i < rules -> pieces; i++)
	{
		j = 0;
		while (default_pieces_moves[i][j] != EMPTY)
			j++;

		rules -> pieces_moves[i] = (int *) malloc(sizeof(int) * (j + 1));
		while (j >= 0)
		{
			rules -> pieces_moves[i][j] = default_pieces_moves[i][j];
			j--;
		}
	}

	rules -> max_avail_moves = default_max_avail_moves;
	rules -> longest_move = default_longest_move;

	return;
}

// set default rules
static void ai_eval_set_default(rules_rules * rules)
{
	int i;
	int j;

	rules -> ai_eval_piece_weight = (int *) 
									malloc(sizeof(int) * rules -> pieces);
	for (i = 0; i < rules -> pieces; i++)
		rules -> ai_eval_piece_weight[i] = default_ai_eval_piece_weight[i];

	rules -> ai_eval_board = (int **) 
							malloc(sizeof(int *) * rules -> board_height);
	for (i = 0; i < rules -> board_height; i++)
	{
		rules -> ai_eval_board[i] = (int *)
								malloc(sizeof(int) * rules -> board_width);
		for (j = 0; j < rules -> board_width; j++)
			rules -> ai_eval_board[i][j] = default_ai_eval_board[i][j];
	}

	for (i = 0; i < 3; i++)
		rules -> ai_bots[i] = default_ai_bots[i];
}

// debugging message
static void rules_debug(char * buf, char * message)
{ 
	if (!buf)
		buf = "<UNKNOWN>";
	char * err_msg = malloc((strlen(buf) + strlen(message) + 12) 
								* sizeof(char));
	err_msg[0] = 0;
	strcat(err_msg, "ERROR at ");
	strcat(err_msg, buf);
	strcat(err_msg, ". ");
	strcat(err_msg, message);
	errors_throw(err_msg);
	free(err_msg);
}

// Write an int or array of ints to a pointer
// Data is retrieved from root if key is null, otherwise a node with
// matching key is found
static int rules_write_int(dataNode * root, char * key, int * pointer)
{
	dataNode * buf = root;
	if (key)
		buf = parser_find(root, key);
	if (!buf)
	{
		rules_debug(key, "Can't find such variable.");
		return 0;
	}
	if (buf -> type == STRUCT)
	{
		rules_debug(key, "Variable type is STRUCT.");
		return 0;
	}

	int i;
	if (buf -> type == INT)
	{
		int * array = (int *) buf -> value;
		for (i = 0; i < buf -> size; i++)
			pointer[i] = array[i];
	}
	else
	{
		char * array = (char *) buf -> value;
		for (i = 0; i < buf -> size; i++)
			pointer[i] = (int) array[i];
	}

	return 1;
}

// Write string or a character to a pointer.
// Data is retrieved from root if key is null, otherwise a node with
// matching key is found.
// If size is zero, all characters are written to string
static int rules_write_char(dataNode * root, char * key, char * pointer, int size)
{
	dataNode * buf = root;
	if (key)
		buf = parser_find(root, key);
	if (!buf)
	{
		rules_debug(key, "Can't find such variable.");
		return 0;
	}
	if (buf -> type == STRUCT)
	{
		rules_debug(key, "Variable type is STRUCT.");
		return 0;
	}

	int i;
	if (!size)
		size = buf -> size;
	if (buf -> type == INT)
	{
		int * array = (int *) buf -> value;
		for (i = 0; i < size; i++)
			pointer[i] = (char) array[i];
		if (size > 1)
			pointer[size] = 0;
	}
	else
	{
		char * array = (char *) buf -> value;
		memcpy(pointer, array, size);
		if (size > 0)
			pointer[size] = 0;
	}

	return 1;
}

//=========================== Public functions ============================

// Set default rules
rules_rules * rules_get_default()
{
	int i;
	rules_rules * rules = (rules_rules *) malloc(sizeof(rules_rules));
	board_set_default(rules);
	moves_set_default(rules);
	ai_eval_set_default(rules);

	rules -> event_count = default_event_count;
	rules -> events = (rules_event *) malloc(sizeof(rules_event) *
												rules -> event_count);
	for (i = 0; i < rules -> event_count; i++)
		rules -> events[i] = default_events[i];
	rules -> description = malloc((strlen(default_description) + 1) * 
									sizeof(char));
	strcpy(rules -> description, default_description);
	return rules;
}

rules_rules * rules_get(char * filepath)
{
	dataNode * root = parser_get(filepath);
	dataNode * buf = 0, * buf2 = 0, ** buf_arr = 0, ** buf_arr2 = 0;
	dataNode ** buf_arr3 = 0;
	int okay_flag = 1, player;
	char * char_array;
	int i, j;

	if (!root)
	{
		rules_debug(filepath, "Can't read data from file.");
		return 0;
	}

	rules_rules * rules = (rules_rules *) calloc(1, sizeof(rules_rules));
	okay_flag = 
		rules_write_int(root, "board_width", &(rules -> board_width)) &
		rules_write_int(root, "board_height", &(rules -> board_height)) &
		rules_write_int(root, "pieces", &(rules -> pieces)) &
		//rules_write_int(root, "player_count", rules -> player_count) &
		rules_write_int(root, "moves_count", &(rules -> moves_count))
		& rules_write_int(root, "max_avail_moves", 
							&(rules -> max_avail_moves));
	if (!okay_flag)
	{
		rules_debug(filepath, "Can't find main values");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}	

	buf = parser_find(root, "sample");
	if (!buf)
	{
		rules_debug(filepath, "Can't find board sample");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}
	if (buf -> size < rules -> board_width)
	{
		rules_debug(filepath, "Sample is too short");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}

	rules -> piece_owners = (char *) malloc(sizeof(char) * rules -> pieces + 1);
	rules -> piece_icon = (char *) malloc(sizeof(char) * rules -> pieces + 1);
	rules -> piece_count = (int *) calloc(rules -> pieces, sizeof(int));
	okay_flag = 
		rules_write_char(root, "piece_owners", rules -> piece_owners, 
						 rules -> pieces) &
		// rules_write_int(root, "piece_count", rules -> piece_count) &
		rules_write_char(root, "piece_icon", rules -> piece_icon,
						 rules -> pieces);

	if (!okay_flag)
	{
		rules_debug(filepath, "Can't find piece data.");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}

	buf_arr = (dataNode **) buf -> value;
	rules -> sample = (char **) calloc(rules -> board_width,
								sizeof(char *));
	rules -> default_index_board = (int **) calloc(rules -> board_width,
								sizeof(int *));

	for (i = 0; i < 3; i++)
	{
		rules -> player_count[i] = 0;
		rules -> default_piece_list[i * 2] = (int *) calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));
		rules -> default_piece_list[i * 2 + 1] = (int *) calloc(
			rules -> board_width * rules -> board_height + 1, sizeof(int));
	}


	for (i = 0; i < rules -> board_width; i++)
	{
		buf = buf_arr[i];
		char_array = (char *) buf -> value;
		rules -> sample[i] = (char *) malloc(
								sizeof(char) * (rules -> board_height + 1));
		rules -> default_index_board[i] = (int *) calloc(
								rules -> board_height + 1, sizeof(int));

		rules_write_char(buf, 0, rules -> sample[i], rules -> board_height);
		for (j = 0; j < rules -> board_height; j++)
		{
			player = rules_get_player(rules, rules -> sample[i][j]);
			if (player != EMPTY)
			{
				// count pieces
				if (player != 2)
					rules -> piece_count[rules -> sample[i][j]]++;

				// save position of each piece
				rules -> default_piece_list[player * 2]
						[rules -> player_count[player]] = i;
				rules -> default_piece_list[player * 2 + 1]
						[rules -> player_count[player]] = j;
				rules -> default_index_board[i][j]
						= rules -> player_count[player]++;
			}
		}
	}


	buf = parser_find(root, "moves");
	if (!buf)
	{
		rules_debug(filepath, "Can't find any moves.");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}
	moves_move * moves = (moves_move *) malloc(sizeof(moves_move) * 
											rules -> moves_count);
	buf_arr = (dataNode **) buf -> value;
	for (i = 0; i < rules -> moves_count; i++)
	{
		buf = buf_arr[i];
		okay_flag = 
			rules_write_char(buf, "type", &(moves[i].type), 1) &
			rules_write_char(buf, "priority", &(moves[i].priority), 1) &
			rules_write_char(buf, "dx", moves[i].dx, 0) &
			rules_write_char(buf, "dy", moves[i].dy, 0) &
			rules_write_char(buf, "repeatable", &(moves[i].repeatable), 1);
		if (!okay_flag)
		{
			rules_debug("moves", "Not enough move properties.");
			free(moves);
			rules_destroy(rules);
			parser_destroy_tree(root);
			return 0;
		}
		buf2 = parser_find(buf, "conditions");
		buf = parser_find(buf, "actions");
		buf_arr2 = (dataNode **) buf2 -> value;
		buf_arr3 = (dataNode **) buf -> value;
		for (j = 0; j < buf2 -> size; j++)
		{
			okay_flag = 
				rules_write_char(buf_arr2[j], 0, 
								moves[i].conditions[j], 0) &
				rules_write_char(buf_arr3[j], 0, 
								moves[i].actions[j], 0);
			if (!okay_flag)
			{
				rules_debug("moves", 
					"Can't write conditions or actions of a move.");
				free(moves);
				rules_destroy(rules);
				parser_destroy_tree(root);
				return 0;
			}
		}
	}
	rules -> moves = moves;

	buf = parser_find(root, "pieces_moves");
	if (!buf)
	{
		rules_debug(filepath, "Can't define moves for pieces.");
		rules_destroy(rules);
		parser_destroy_tree(root);
		return 0;
	}
	rules -> pieces_moves = (int **) calloc(rules -> pieces, sizeof(int *));
										
	buf_arr = (dataNode **) buf -> value;
	for (i = 0; i < rules -> pieces; i++)
	{
		rules -> pieces_moves[i] = (int *) malloc(sizeof(int) *
												  buf_arr[i] -> size);
		okay_flag = rules_write_int(buf_arr[i], 0, 
									rules -> pieces_moves[i]);
		if (!okay_flag)
		{
			rules_debug("pieces_moves", "Can't define moves for a piece.");
			rules_destroy(rules);
			parser_destroy_tree(root);
			return 0;
		}
	}

	okay_flag = rules_write_int(root, "event_count", 
								&(rules -> event_count));
	if (!okay_flag)
	{
		rules_debug(filepath, "No event support. Continuing.");
		okay_flag = 1;
	}
	else
	{
		buf = parser_find(root, "events");
		buf_arr = (dataNode **) buf -> value;
		rules -> events = (rules_event *) malloc(sizeof(rules_event) *
												rules -> event_count);
		for (i = 0; i < rules -> event_count; i++)
		{
			okay_flag =
				rules_write_char(buf_arr[i], "con", 
								 rules -> events[i].con, 0) &
				rules_write_char(buf_arr[i], "act", 
								 rules -> events[i].act, 0);
			if (!okay_flag)
			{
				rules_debug("events", "Can't read events.");
				rules_destroy(rules);
				parser_destroy_tree(root);
				return 0;
			}
		}
	}
	
	rules -> ai_eval_piece_weight = (int *) malloc(sizeof(int) *
													rules -> pieces);
	okay_flag = rules_write_int(root, "ai_eval_piece_weight",
								rules -> ai_eval_piece_weight);
	if (!okay_flag)
	{
		okay_flag = 1;
		rules_debug(filepath, 
			"Can't find pieces weight for AI. Continuing.");
	}

	buf = parser_find(root, "ai_eval_board");
	if (!buf)
		rules_debug(filepath,
			"Can't find position weight for AI. Continuing.");
	else
	{
		rules -> ai_eval_board = (int **) calloc(rules -> board_width,
												sizeof(int*));
		buf_arr = (dataNode **) buf -> value;
		for (i = 0; i < rules -> board_width; i++)
		{
			rules -> ai_eval_board[i] = (int *) calloc(
						rules -> board_height, sizeof(int));
			okay_flag = rules_write_int(buf_arr[i], 0 ,
										rules -> ai_eval_board[i]);
			if (!okay_flag)
			{
				rules_debug("ai_eval_board", "Can't read.");
				rules_destroy(rules);
				parser_destroy_tree(root);
				return 0;
			}
		}
	}

	okay_flag = rules_write_int(root, "ai_bots", rules -> ai_bots);
	if (!okay_flag)
	{
		rules_debug("ai_bots", "Set to default");
		for (i = 0; i < 3; i++)
			rules -> ai_bots[i] = default_ai_bots[i];
	}

	okay_flag = rules_write_int(root, "longest_move", 
							&(rules -> longest_move));
	if (!okay_flag)
	{
		rules_debug("longest_move", "Set to default.");
		rules -> longest_move = default_longest_move;
	}

	buf = parser_find(root, "description");
	if (!buf)
	{
		rules_debug("description", "Continuing.");
		rules -> description = 0;
	}
	else
	{
		rules -> description = malloc((buf -> size + 2) * sizeof(char));
		rules_write_char(buf, 0, rules -> description, 0);
	}

	parser_destroy_tree(root);
	return rules;
}

// destroy rules
void rules_destroy(rules_rules * rules)
{
	if (!rules)
		return;
	int i;
	if (rules -> ai_eval_board)
	{
		for (i = 0; i < rules -> board_width; i++)
			if (rules -> ai_eval_board[i])
				free(rules -> ai_eval_board[i]);
		free(rules -> ai_eval_board);
	}

	free(rules -> ai_eval_piece_weight);
	free(rules -> events);
	free(rules -> moves);

	if (rules -> pieces_moves)
	{
		for (i = 0; i < rules -> pieces; i++)
			if (rules -> pieces_moves[i])
				free(rules -> pieces_moves[i]);
		free(rules -> pieces_moves);
	}

	free(rules -> piece_icon);
	free(rules -> piece_owners);
	free(rules -> piece_count);
	free(rules -> description);

	if (rules -> sample)
	{
		for (i = 0; i < rules -> board_width; i++)
			free(rules -> sample[i]);
		free(rules -> sample);
	}
	
	if (rules -> default_index_board)
	{
		for (i = 0; i < rules -> board_width; i++)
			free(rules -> default_index_board[i]);
		free(rules -> default_index_board);
	}

	for (i = 0; i < 6; i++)
		if (rules -> default_piece_list[i])
			free(rules -> default_piece_list[i]);

	free(rules);
}

int rules_get_player(rules_rules * rules, char piece)
{
	switch(piece)
	{
		case EMPTY:
			return EMPTY;
			break;

		case WALL: case TEMP:
			return 2;
			break;

		default:
			return rules -> piece_owners[piece];
			break;
	}
}
