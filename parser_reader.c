#include "parser.h"
#include <stdlib.h>


static int parser_debug_line = 1;
static const int buffer_size = 100;

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
					return 0;
				}
				else if (stack[stack_top--] != LSB)
				{	
					parser_debug(file, "Can't parse file", 0);
					parser_destroy_stream(result);
					return 0;
				}
				break;

			case RAB:
				if (0 > stack_top)
				{
					parser_debug(file, "Can't parse file", line);
					parser_destroy_stream(result);
					return 0;
				}
				else if (stack[stack_top--] != LAB)
				{	
					parser_debug(file, "Can't parse file", line);
					parser_destroy_stream(result);
					return 0;
				}
				break;
		}
		buf = buf -> next;
	}
	buf -> next = 0;
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
