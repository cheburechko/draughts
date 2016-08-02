#include <stdio.h>
#include "macros.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

// Buffer sizes
static const int buffer_size = 100;
static const int max_struct_size = 100;
static const int max_array_size = 100;

// The ammount of macroses
static const int parser_macros_count = 41;

// An array of macroses keys
static const char parser_macros_keys[50][20] = {
	"CON_END", "CON_START", "CON_ENEMY", "CON_DEFAULT", "CON_EMPTY",
	"CON_ALLY", "CON_OBS", "CON_COUNT", "CON_PLAYER", "CON_NO_MOVES",
	"CON_PIECE", "CON_COORD", "CON_BOOL", "CON_NUM", "CON_TARGET",
	"CON_ANY", "CON_LINE", "CON_LAST_PLAYER", "CON_CUR_PLAYER",
	"CON_COLUMN", "CON_LR_DIAG", "CON_RL_DIAG", "CON_COUNTER",
	"CON_OFFSET", "CON_BYTE",

	"OP_EQUAL", "OP_INEQUAL", "OP_AND", "OP_OR",
	
	"ACT_REPLACE", "ACT_ABORT", "ACT_REPLACE_OBS", "ACT_REPEAT", "ACT_NEXT", 
	"ACT_CONVERT", "ACT_INC_COUNTER", "ACT_DEC_COUNTER", "ACT_REMOVE",

	"TEMP", "WALL", "EMPTY"
};

// An array of macroses values
static const int parser_macros_values[] = {
	CON_END, CON_START, CON_ENEMY, CON_DEFAULT, CON_EMPTY,
	CON_ALLY, CON_OBS, CON_COUNT, CON_PLAYER, CON_NO_MOVES,
	CON_PIECE, CON_COORD, CON_BOOL, CON_NUM, CON_TARGET,
	CON_ANY, CON_LINE, CON_LAST_PLAYER, CON_CUR_PLAYER,
	CON_COLUMN, CON_LR_DIAG, CON_RL_DIAG, CON_COUNTER,
	CON_OFFSET, CON_BYTE,

	OP_EQUAL, OP_INEQUAL, OP_AND, OP_OR,
	
	ACT_REPLACE, ACT_ABORT, ACT_REPLACE_OBS, ACT_REPEAT, ACT_NEXT, 
	ACT_CONVERT, ACT_INC_COUNTER, ACT_DEC_COUNTER, ACT_REMOVE,

	TEMP, WALL, EMPTY
};

// dataNode destructor
static void parser_destroy(dataNode * node)
{
	free(node -> name);
	free(node -> value);
	free(node);
}

// flags
static const int PARSER_NAMELESS = 1;
static int parser_debug_line = 1;

// debugging
static void parser_debug(char * buf, char * message, int line)
{
	if (buf == 0)
		buf = "<UNKNOWN>";
	char line_str[10];
	int i = 7;
	line_str[9] = 0;
	line_str[8] = line % 10 + '0';
	line /= 10;
	while (line != 0)
	{
		line_str[i--] = line % 10 + '0';
		line /= 10;
	}
	char * err_msg = (char *) malloc(
			(strlen(buf) + strlen(message) + 18 + 8 - i) * sizeof(char));
	err_msg[0] = 0;
	strcat(err_msg, "ERROR at ");
	strcat(err_msg, line_str + i + 1);
	strcat(err_msg, " with ");
	strcat(err_msg, buf);
	strcat(err_msg, ": ");
	strcat(err_msg, message);
	errors_throw(err_msg);
	free(err_msg);
}

//============ Working with buffer ========================================

static parser_buffer * parser_create_buffer(int size, char * file)
{
	parser_buffer * result = (parser_buffer *) 
								malloc(sizeof(parser_buffer));
	result -> input = fopen(file, "r");
	if (!result -> input)
	{
		free(result);
		parser_debug(file, "Can't open file", 0);
		return 0;
	}
	result -> size = size;
	result -> pos = 0;
	result -> end = 0;
	result -> buf = malloc(sizeof(char) * (size + 1));
	result -> size = fread(result -> buf, 
		sizeof(char), result -> size, result -> input);
	if (feof(result -> input))
		result -> end = 1;
	return result;
}

static int parser_buffer_is_empty(parser_buffer * buffer)
{
	return buffer -> pos >= buffer -> size && buffer -> end;
}

// read next buffer_szie bytes to buffer
static void parser_read(parser_buffer * buffer)
{
	if (buffer -> pos < buffer -> size || buffer -> end)
		return;

	buffer -> pos = 0;
	buffer -> size = fread(buffer -> buf, sizeof(char), 
							buffer -> size, buffer -> input);
	buffer -> buf[buffer -> size] = 0;
	if (feof(buffer -> input))
		buffer -> end = 1;
}

static void parser_destroy_buffer(parser_buffer * buffer)
{
	free(buffer -> buf);
	fclose(buffer -> input);
	free(buffer);
}


// skip trash
static int parser_skip_trash(parser_buffer * buffer)
{
	int flag = 1;
	char c;
	do
	{
		parser_read(buffer);
		if (parser_buffer_is_empty(buffer))
			return parser_debug_line;
		c = buffer -> buf[buffer -> pos++];
		if (c == '=' || c == '[' || c == ']' || c == '{' || c == '}' ||
			c == '"' || c == 39  || c == '_' || c >= '0' && c <= '9' ||
			c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '-')
			flag = 0;
		if (c == '\n')
			parser_debug_line++;
	} while (flag);

	buffer -> pos--;
	return parser_debug_line;
}

// Find macros value. Returns 0 if given macros is not found.
static int parser_find_macros(char * macros)
{
	int i;
	int result = 0;
	for (i = 0; i < parser_macros_count; i++)
		if (!strcmp(macros, parser_macros_keys[i]))
		{
			result = parser_macros_values[i];
			break;
		}
	return result;
}


// Converts to integer. Dest must be initialized!
static int parser_convert_to_int(char * buf, int * dest, int line)
{
	int neg_flag = 1;
	*dest = 0;
	int i = 0;

	switch (buf[0])
	{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '-':
			// remember a minus
			if (buf[i] == '-')
			{
				neg_flag = -1;
				i++;
			}	
			while (buf[i])
			{
				if (buf[i] < '0' || buf[i] > '9')
				{
					parser_debug(buf, "Can't convert to integer.", line);
					return 0;
				}
				*dest = *dest * 10 + (buf[i++] - '0');
			}
			*dest *= neg_flag;
			return 1;
			break;
		// try to get a macros value if input buf is not int
		default:
			*dest = parser_find_macros(buf);

			if (!*dest)
			{
				parser_debug(buf, "Can't find macros.", line);
				return 0;
			}

			return 1;
			break;
	}
	// If no value gained, an error occurs
	parser_debug(buf, "Can't convert to integer.", line);
	return 0;
}

// Converts to string. Dest must be initialized!
static int parser_convert_to_string(char * buf, char * dest, int line)
{
	int len;

	switch(buf[0])
	{
		// single character
		case '\'':
			*dest = buf[1];
			return 1;
			break;
		// string
		case '"':
			len = strlen(buf);
			strncpy(dest, buf + 1, len - 2);
			dest[len - 2] = 0;
			return len - 1;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			// convert ASCII code to char
			if (!parser_convert_to_int(buf, &len, line))
			{
				parser_debug(buf, "Can't convert int to char", line);
				return 0;
			}
			
			if (len > 255)
			{
				strcpy(dest ,buf);
				return strlen(buf);
			}
			else
			{
				*dest = (char) len;
				return 1;
			}

			break;
		// convert macros value to char
		default:
			*dest = parser_find_macros(buf);

			if (!*dest)
			{
				parser_debug(buf, "Can't find macros.", line);
				return 0;
			}

			return 1;
			break;
	}
	// An error occurs if no value found
	parser_debug(buf, "Can't convert to string.", line);
	return 0;
}

// get new word from file
static dataStream * parser_read_next(parser_buffer * buffer, int * line)
{
	if (parser_buffer_is_empty(buffer))
	{
		parser_debug(0, "Unexpected end of file", *line);
		return 0;
	}
	*line = parser_skip_trash(buffer);
	dataStream * stream = (dataStream *) malloc(sizeof(dataStream));
	stream -> next = 0;

	parser_read(buffer);
	char * result = (char *) malloc(sizeof(char) * buffer_size);
	result[0] = buffer -> buf[buffer -> pos++];
	result[1] = 0;
	int len = 1, size = buffer_size;
	int okay_flag = 1;
	char c;
	parser_read(buffer);

	switch(result[0])
	{
		// structure and array initializers and '='
		case '{': case '}': case '=': case '[': case ']':
			stream -> token = result[0];
			stream -> value = 0;
			stream -> size = 1;
			free(result);
			break;

		// single character
		case 39: //'
			stream -> token = CH;
			result[0] = 0;
			len = 0;
			while (buffer -> buf[buffer -> pos] != '\'')
			{
				if (len + 2 >= size)
				{
					size *= 2;
					result = realloc(result, size);
				}
				result[len++] = buffer -> buf[buffer -> pos++];
				parser_read(buffer);
			}
			if (len > 1)
			{
				parser_debug(result, "Too long to convert to character.",
							 *line);
				free(result);
				free(stream);
				return 0;
			}
			buffer -> pos++;
			result[len] = 0;
			stream -> value = result;
			stream -> size = 1;
			break;
		// string
		case '"':
			result[0] = 0;
			len = 0;
			while (buffer -> buf[buffer -> pos] != '\"')
			{
				if (len + 2 >= size)
				{
					size *= 2;
					result = realloc(result, size);
				}
				result[len++] = buffer -> buf[buffer -> pos++];
				parser_read(buffer);
			}
			buffer -> pos++;
			result[len] = 0;
			stream -> token = STR;
			stream -> value = result;
			stream -> size = len;
			break;
		// macros and ints
		default:
			c = buffer -> buf[buffer -> pos];
			while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
				   (c >= '0' && c <= '9') ||  c == '-' || c == '_')
			{
				buffer -> pos++;
				if (len + 1 >= size)
				{
					size *= 2;
					result = realloc(result, size);
				}
				result[len++] = c;
				parser_read(buffer);
				c = buffer -> buf[buffer -> pos];
			}
			result[len] = 0;
			if (result[0] <= '9' && result[0] >= '0' || result[0] == '-')
			{
				stream -> value = malloc(sizeof(int));
				stream -> token = IN;
				stream -> size = 1;
				okay_flag = parser_convert_to_int
								(result, (int *) stream -> value, *line);
				if (!okay_flag)
				{
					parser_debug(result, "Can't parse variable", *line);
					free(result);
					free(stream -> value);
					free(stream);
					return 0;
				}
				free(result);
			}
			else
			{
				stream -> value = result;
				stream -> size = len;
				stream -> token = MAC;
			}
			break;

	}
	return stream;
}

void parser_destroy_stream(dataStream * stream)
{
	if (!stream)
		return;
	free(stream -> value);
	parser_destroy_stream(stream -> next);
	if (stream -> token != ROOT)
		free(stream);
	else
		stream -> next = 0;
}

dataStream * parser_parse(char * file)
{
	parser_buffer * buffer = parser_create_buffer(buffer_size, file);
	if (!buffer)
	{
		parser_debug(file, "Can't open file", 0);
		return 0;
	}
	dataStream * result = (dataStream *) calloc(1, sizeof(dataStream));
	char stack[1000] = {0};
	int stack_top = -1, line = 1;
	result -> token = ROOT;
	dataStream * buf = result;
	char * word;
	while (!parser_buffer_is_empty(buffer))
	{
		buf -> next = parser_read_next(buffer, &line);
		if (!buf -> next)
		{
			parser_debug(file, "Can't parse file", line);
			parser_destroy_stream(result);
					parser_destroy_buffer(buffer);
			return 0;
		}

		switch(buf -> token)
		{
			case LSB: case LAB:
				stack[++stack_top] = buf -> token;
				break;

			case RSB:
				if (0 > stack_top)
				{
					parser_debug(file, "Can't parse file", line);
					parser_destroy_stream(result);
					parser_destroy_buffer(buffer);
					return 0;
				}
				else if (stack[stack_top--] != LSB)
				{	
					parser_debug(file, "Can't parse file", 0);
					parser_destroy_stream(result);
					parser_destroy_buffer(buffer);
					return 0;
				}
				break;

			case RAB:
				if (0 > stack_top)
				{
					parser_debug(file, "Can't parse file", line);
					parser_destroy_stream(result);
					parser_destroy_buffer(buffer);
					return 0;
				}
				else if (stack[stack_top--] != LAB)
				{	
					parser_debug(file, "Can't parse file", line);
					parser_destroy_stream(result);
					parser_destroy_buffer(buffer);
					return 0;
				}
				break;
		}
		buf = buf -> next;
	}
	buf -> next = 0;
	parser_destroy_buffer(buffer);
	return result;
	
}

dataStream * parser_get_next(dataStream * stream)
{
	dataStream * result = stream -> next;
	if (result)
	{
		stream -> next = result -> next;
		result -> next = 0;
	}
	return result;
}

dataStream * parser_look_next(dataStream * stream)
{
	return stream -> next;
}
// get data from file and convert it to dataNode
dataNode * parser_get_data(dataStream * stream, int flag)
{
	// Initializing node
	dataNode * result = (dataNode *) malloc(sizeof(dataNode));
	result -> value = 0;
	result -> size = 0;
	result -> name = 0;

	// Initializing flags and buffers
	int err_flag = 1;
	dataStream * buf = 0;
	dataNode ** struct_array = 0;
	int * int_array = 0;
	char * char_array = 0;
	int size;
	int line = parser_debug_line, line2 = parser_debug_line;
	
	// Get name if needed
	if (!(flag & PARSER_NAMELESS))
	{
		buf = parser_get_next(stream);
		if (!buf)
		{
			parser_debug(0, "Unexpected end of stream.", line);
			parser_destroy_stream(stream);
			free(result);
			return 0;
		}
		if (buf -> token != MAC)
		{
			parser_debug(0, "Name should start with a-z.", line);
			parser_destroy_stream(buf);
			parser_destroy_stream(stream);
			free(result);
			return 0;
		}
		result -> name = buf -> value;
		free(buf);
		// name must be followed by '='
		buf = parser_get_next(stream);
		if (!buf)
		{
			parser_debug(0, "Unexpected end of stream.", line);
			free(result -> name);
			parser_destroy_stream(stream);
			free(result);
			return 0;
		}
		if (buf -> token != EQ)
		{
			parser_debug(0, "No '=' found.", line);
			free(result -> name);
			free(result);
			parser_destroy_stream(buf);
			parser_destroy_stream(stream);
			return 0;
		}
		parser_destroy_stream(buf);
	}

	// Get value
	buf = parser_get_next(stream);
	if (!buf)
	{
		parser_debug(0, "Unexpected end of stream.", line2);
		parser_destroy_stream(stream);
		if (!(flag & PARSER_NAMELESS))
			free(result -> name);
		free(result);
		return 0;
	}
	switch(buf -> token)
	{
		// structure
		case LSB:
			result -> type = STRUCT;
			size = buffer_size;
			struct_array = (dataNode **) malloc(sizeof(dataNode *) * size);
			
			parser_destroy_stream(buf);
			// get children until structure finishes
			while (stream -> next -> token != RSB)
			{
				// check size
				if (result -> size >= size)
				{
					size *= 2;
					struct_array = realloc(struct_array, size);
				}

				struct_array[result -> size++] = parser_get_data(stream, 0);

				// check for errors
				if (!struct_array[result -> size - 1])
				{
					parser_debug(result -> name, "Can't read structure.",
								 line);
					err_flag = 0;
					break;
				}
			}
			result -> value = struct_array;
			parser_destroy_stream(parser_get_next(stream));
			break;

		// an array (tricky part)
		case LAB:
			
			parser_destroy_stream(buf);
			// define an array type
			switch (stream -> next -> token)
			{
				// arrays should not be empty coz no one needs them
				case RAB:
					parser_debug(0, "Empty array.", line);
					err_flag = 0;
					break;

				// create an array of different structures
				// such as another arrays or structs
				// works like struct but the children don't have names
				// ? It may be possible to put children of different types
				case LSB: case LAB: case STR:
					// Array of arrays or structures
					result -> type = STRUCT;
					size = buffer_size;
					struct_array = (dataNode **) malloc(
								sizeof(dataNode *) * size);

					while (stream -> next -> token != RAB)
					{
						// check the size
						if (result -> size >= size)
						{
							size *= 2;
							result = realloc(result, size);
						}
						struct_array[result -> size++] = 
								parser_get_data(stream, PARSER_NAMELESS);
						// check errors
						if (!struct_array[result -> size - 1])
						{
							parser_debug(result -> name, 
								"Can't read array structure member", line);
							err_flag = 0;
							break;
						}

					}
					parser_destroy_stream(parser_get_next(stream));
					result -> value = struct_array;
					break;

				// integer/char/macros array
				default:
					result -> type = INT;
					size = buffer_size;
					int_array = (int *) 
							malloc(sizeof(int) * size);
					
					while (stream -> next -> token != RAB)
					{
						// check size
						if (result -> size >= size)
						{
							size *= 2;
							result = realloc(result, size);
						}

						buf = parser_get_next(stream);

						if (!buf)
						{
							parser_debug(0, "Read error.", line);
							err_flag = 0;
							break;
						}
						if (buf -> token == MAC)
						{
							int_array[result -> size++] = 
								parser_find_macros((char *) buf -> value);
							if (int_array[result -> size - 1] == 0)
								err_flag = 0;
						}
						else
							int_array[result -> size++] = *(int *)
													(buf -> value);

						parser_destroy_stream(buf);
						// check errors
						if (!err_flag)
						{
							parser_debug(result -> name,
									"Can't read array int member", line2);
							break;
						}
					}
					parser_destroy_stream(parser_get_next(stream));
					result -> value = int_array;
					break;	
			}
			break;
		
		// string
		case STR:
			result -> type = CHAR;
			result -> size = buf -> size;
			result -> value = buf -> value;
			free(buf);
			break;

		default:
			result -> type = INT;
			result -> size = 1;
			result -> value = malloc(sizeof(int));
			if (buf -> token == MAC)
			{
				*(int *)(result -> value) = 
						parser_find_macros(buf -> value);
				if (!(result -> value))
					err_flag = 0;
			}
			else 
				*(int *)(result -> value) = *(int *) (buf -> value);
			parser_destroy_stream(buf);
			break;
	}

	// check errors
	if (!err_flag)
	{
		parser_debug(result -> name, "Can't read variable.", line);
		parser_destroy_tree(result);
		parser_destroy_stream(stream);
		return 0;
	}

	return result;
}

//======================== Public functions ===============================

// Create a tree of data from file. 
dataNode * parser_get(char * filepath)
{
	parser_debug_line = 1;
	if (!filepath)
		return 0;
	dataStream * input = parser_parse(filepath);
	if (input)
	{
		dataNode * result = parser_get_data(input, 0);
		parser_destroy_stream(input);
		free(input);
		return result;
	}
	else
		return 0;
}

// Destroy a tree. Destroys all the pointers, so the data from tree
// must be copied for oncoming use.
void parser_destroy_tree(dataNode * root)
{
	if (!root)
	 	return;
	if (root -> type == STRUCT || root -> type == ARRAY_STRUCT)
	{
		int i;
		dataNode ** buf = (dataNode **) root -> value;
		for (i = 0; i < root -> size; i++)
			parser_destroy_tree(buf[i]);
	}
	parser_destroy(root);
	return;
}

// Find a node by its name
dataNode * parser_find(dataNode * root, char * key)
{
	if (root -> name)
		if (!strcmp(root -> name, key))
			return root;
	
	dataNode * result = 0, **buf;
	if (root -> type == STRUCT || root -> type == ARRAY_STRUCT)
	{
		int i;
		buf = (dataNode **) root -> value;
		for (i = 0; i < root -> size; i++)
		{
			result = parser_find(buf[i], key);
			if (result)
				break;
		}
	}

	return result;
}
