#ifndef __draughts_parser_h
#define __draughts_parser_h

#include <stdio.h>

enum dataType { INT, STRUCT, ARRAY_STRUCT, CHAR };

enum tokenType { LSB = '{', RSB = '}', LAB = '[', RAB = ']', EQ = '=',
				 CH = 0, IN, MAC, STR, ROOT};

typedef struct dataNode
{
	enum dataType type;
	char * name;
	int size;
	void * value;
} dataNode;

typedef struct dataStream
{
	void * value;
	enum tokenType token;
	struct dataStream * next;
	int size;
} dataStream;

typedef struct parser_buffer
{
	int size, pos, end;
	char * buf;
	FILE * input;
} parser_buffer;

/*
	How does it work:

	Parser can read four types of data:

	* INT - a single integer or an array of integers
	* CHAR - the same but with chars
	* STRUCT - structure that has children data of differen types
	* ARRAY_STRUCT - obsolete

	Variables are declared like these

	var_name = value

	var_name must begin with a..z and can contain a..z, A..Z, 0..9 and _

	A sequence of 0..9 and - is treated as a single integer

	'@' - a single character. Can contain any character except '.

	"string^^" - a string. Can contain any charcters except ".

	some_Macr0s - a macros. By default its value is treated as a character,
	though it can be used in int arrays

	{...} - a structure. Can contain another variables. Vars
	can be of different types. All of them must be
	declared as a usual var.
	Example:
	root = {
		seven = 7 string = "string" char = 'c' mac = EMPTY
	}

	[...] - an array. Can contain another variables but those must not have
	names and they are only declared by their values.
		* The difference between structure and arrays is that vars
		in arrays must not have any names and should be of the 
		same type. 
		* Type of an array is defined by the first element found in array.
		* Character array may contain single characters, macroses and
		positive integers, whose value is treated as ASCII code.
		* Int array can have ints and macroses.
		* Struct arrays can have structs and another arrays. For parser
		there is no real difference between them.
	
	parser_get takes the first var from the file, everything else
	is ignored. So a file should have root structure.

	List of permitted chars: a..z A..Z 0..9 _ = { } [ ] - " '
	Everything else is treated as delimeters.
*/

dataStream * parser_parse(char * file);

dataStream * parser_get_next(dataStream * stream);

void parser_destroy_stream(dataStream *stream);

// Gets data from file in the filepath and builds a tree.
dataNode * parser_get(char * filepath);

// Find a node by its name
dataNode * parser_find(dataNode * root, char * key);



// destructor
void parser_destroy_tree(dataNode * root);

#endif
