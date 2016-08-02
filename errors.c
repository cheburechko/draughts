#include <stdlib.h>
#include <string.h>

enum {errors_max_buffer_size = 100};

static char * errors_buffer[errors_max_buffer_size];

static int errors_msgs_count = 0;

static int errors_total_length = 0;

void errors_throw(char * err_msg)
{
	if (errors_msgs_count < errors_max_buffer_size)
	{
		int length = strlen(err_msg) + 1;
		errors_total_length += length;
		errors_buffer[errors_msgs_count] = (char *)  malloc
											((length + 1) * sizeof(char));
		strcpy(errors_buffer[errors_msgs_count], err_msg);
		errors_buffer[errors_msgs_count][length - 1] = '\n';
		errors_buffer[errors_msgs_count++][length] = 0;	
	}
}

char * errors_get()
{
	char * result = malloc((errors_total_length + 1) * sizeof(char));
	result[0] = 0;
	int i = 0;
	while (i < errors_msgs_count)
	{
		strcat(result, errors_buffer[i]);
		free(errors_buffer[i++]);
	}
	errors_total_length = 0;
	errors_msgs_count = 0;
	return result;
}

char ** errors_get_separate()
{
	char ** result = malloc((errors_msgs_count + 1) * sizeof(char *));
	int i;
	for (i = 0; i < errors_msgs_count; i++)
	{
		result[i] = (char *) malloc
			((strlen(errors_buffer[i]) + 1) * sizeof(char));
		strcpy(result[i], errors_buffer[i]);
		free(errors_buffer[i]);
	}
	result[errors_msgs_count] = 0;
	errors_total_length = 0;
	errors_msgs_count = 0;
	return result;
}
