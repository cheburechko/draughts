# errors test
OBJ_ERRORS = errors.o

# parser test
OBJ_PARSER = parser.o $(OBJ_ERRORS)

# rules test
OBJ_RULES = rules.o $(OBJ_PARSER)

# board test
OBJ_BOARD = board.o $(OBJ_RULES)

# logics test
OBJ_LOGICS = logics.o $(OBJ_BOARD)

# movest test
OBJ_MOVES = moves.o $(OBJ_LOGICS)

# interface test
OBJ_INTER = interface.o gui_board.o gui_info_windows.o gui_menus.o gui_status_lines.o $(OBJ_MOVES)

# ai
OBJ_AI = ai.o $(OBJ_INTER)

BIN_ERRORS_TEST = test_errors
BIN_PARSER_TEST = test_parser
BIN_RULES_TEST = test_rules
BIN_BOARD_TEST = test_board
BIN_LOGICS_TEST = test_logics
BIN_INTER_TEST = test_interface
BIN_MOVES_TEST = test_moves
BIN_BENCH = benchmark

# all objects
OBJ = $(OBJ_AI)


# compilation target
CC = gcc
BIN = draughts
BIN_NOGUI = no_gui

# all binaries
BINS = $(BIN) $(BIN_NOGUI) $(BIN_INTER_TEST) $(BIN_LOGICS_TEST) $(BIN_BOARD_TEST) $(BIN_RULES_TEST) $(BIN_PARSER_TEST) $(BIN_ERRORS_TEST) $(BIN_MOVES_TEST) $(BIN_BENCH)

# debug flag
DEBUG = -g

LIBS = -lmenu -lncurses

# common flags
FLAGS = -Werror

# flags for compilation object files
CFLAGS = $(FLAGS) -c $(DEBUG)

# flags for linking
LFLAGS = $(FLAGS) $(DEBUG)

# target
draughts: $(OBJ) draughts.o
	$(CC) $(LFLAGS) -o $(BIN) $(OBJ) draughts.o $(LIBS)

test_parser: $(OBJ_PARSER) test_parser.o
	$(CC) $(LFLAGS) -o $(BIN_PARSER_TEST) $(OBJ_PARSER) test_parser.o
	
test_rules: $(OBJ_RULES) test_rules.o
	$(CC) $(LFLAGS) -o $(BIN_RULES_TEST) $(OBJ_RULES) test_rules.o

test_board: $(OBJ_BOARD) test_board.o
	$(CC) $(LFLAGS) -o $(BIN_BOARD_TEST) $(OBJ_BOARD) test_board.o

test_logics: $(OBJ_LOGICS) test_logics.o
	$(CC) $(LFLAGS) -o $(BIN_LOGICS_TEST) $(OBJ_LOGICS) test_logics.o

test_interface: $(OBJ_INTER) test_interface.o
	$(CC) $(LFLAGS) -o $(BIN_INTER_TEST) $(OBJ_INTER) test_interface.o $(LIBS) 

test_errors: $(OBJ_ERRORS) test_errors.o
	$(CC) $(LFLAGS) -o $(BIN_ERRORS_TEST) $(OBJ_ERRORS) test_errors.o

test_moves: $(OBJ_MOVES) test_moves.o
	$(CC) $(LFLAGS) -o $(BIN_MOVES_TEST) $(OBJ_MOVES) test_moves.o

no_gui: $(OBJ) no_gui.o
	$(CC) $(LFLAGS) -o $(BIN_NOGUI) $(OBJ) no_gui.o $(LIBS)

benchmark: $(OBJ) benchmark.o
	$(CC) $(LFLAGS) -o $(BIN_BENCH) $(OBJ) benchmark.o $(LIBS)


# object files
board.o: board.c board.h
	$(CC) $(CFLAGS) board.c

logics.o: logics.c logics.h
	$(CC) $(CFLAGS) logics.c

moves.o: moves.c moves.h
	$(CC) $(CFLAGS) moves.c

rules.o: rules.c rules.h
	$(CC) $(CFLAGS) rules.c

interface.o: interface.c interface.h
	$(CC) $(CFLAGS) interface.c

gui_board.o: gui_board.c interface.h
	$(CC) $(CFLAGS) gui_board.c

gui_menus.o: gui_menus.c interface.h
	$(CC) $(CFLAGS) gui_menus.c

gui_info_windows.o: gui_info_windows.c interface.h
	$(CC) $(CFLAGS) gui_info_windows.c

gui_status_lines.o: gui_status_lines.c interface.h
	$(CC) $(CFLAGS) gui_status_lines.c

draughts.o: draughts.c
	$(CC) $(CFLAGS) draughts.c

ai.o: ai.c ai.h
	$(CC) $(CFLAGS) ai.c

parser.o: parser.c parser.h macros.h
	$(CC) $(CFLAGS) parser.c

errors.o: errors.c errors.h
	$(CC) $(CFLAGS) errors.c

# Test object files
test_parser.o: test_parser.c
	$(CC) $(CFLAGS) test_parser.c

test_rules.o: test_rules.c
	$(CC) $(CFLAGS) test_rules.c

test_board.o: test_board.c
	$(CC) $(CFLAGS) test_board.c

test_logics.o: test_logics.c
	$(CC) $(CFLAGS) test_logics.c

test_interface.o: test_interface.c
	$(CC) $(CFLAGS) test_interface.c

test_errors.o: test_errors.c
	$(CC) $(CFLAGS) test_errors.c

test_moves.o: test_moves.c
	$(CC) $(CFLAGS) test_moves.c

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) benchmark.c

no_gui.o: no_gui.c
	$(CC) $(CFLAGS) no_gui.c

# clean
clean:
	rm -f *.o $(BINS)
