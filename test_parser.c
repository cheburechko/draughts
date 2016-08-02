#include "parser.h"
#include "errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print(dataNode * node)
{
	int i;
	char * s;
	int * x;
	printf("Name: ");
	if (node -> name)
		printf("%s\n", node -> name);
	else
		printf("<UNKNOWN>\n");
	switch (node -> type)
	{
		case INT:
			printf("It's an int array of size %d\n", node -> size);
			x = (int *) node -> value;
			for (i = 0; i < node -> size; i++)
				printf("%d int : %d	", i, x[i]);
			printf("\n");
	 		break;
		
		case CHAR:
			printf("It's a char array of size %d\n", node -> size);
			s = (char *) node -> value;
			for (i = 0; i < node -> size; i++)
				printf("%d char : %c	", i, s[i]);
			printf("\n");
			break;

		case STRUCT:
			printf("It's a structure\n");
			break;

		case ARRAY_STRUCT:
			printf("It's an array of structures\n");
			break;
	}

}

int main(int argc, char ** argv)
{
	if (argc < 2)
		return 0;
	dataNode * root = parser_get(argv[1]);

	char * s = errors_get();
	printf("%s", s);
	free(s);
	
	if (!root)
		return 0;

	dataNode * buf = root, * buff, ** array;
	printf("Tree successfully built\n");
	char input[100] = "";
	int int_input = 0;
	while (strcmp(input, "exit"))
	{
		scanf("%s", input);
		if (!strcmp(input, "find"))
		{
			printf("Type the name:");
			scanf("%s", input);
			buff = parser_find(buf, input);
			if (buff)
			{
				printf("Found!\n");
				buf = buff;
			}
			else
				printf("Not found!\n");
		} 
		else if (!strcmp(input, "print"))
		{
			print(buf);
		}
		else if (!strcmp(input, "root"))
		{
			printf("Moved to root\n");
			buf = root;
		}
		else if (!strcmp(input, "child"))
		{
			if (buf -> type == INT || buf -> type == CHAR)
				printf("This node can't have children!!!\n");
			else
			{
				printf("Array size = %d. Type index (>= 0):", buf -> size);
				scanf("%d", &int_input);
				if (int_input < 0 || int_input >= buf -> size)
					printf("Index out of bounds\n");
				else
				{
					array = (dataNode **) (buf -> value);
					buf = array[int_input];
					printf("Found!\n");
				}
			}
		}
		else
			printf("Unknown command\n");
	}
	parser_destroy_tree(root);
}
