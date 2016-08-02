#include "errors.h"
#include <stdio.h>

int main()
{
	errors_throw("Error 1");
	errors_throw("Error 2");
	errors_throw("Error 3");
	errors_throw("Error 4");
	errors_throw("Error 5");
	char * s = errors_get();
	printf("%s", s);
	free(s);

	errors_throw("Error 1");
	errors_throw("Error 2");
	errors_throw("Error 3");
	errors_throw("Error 4");
	errors_throw("Error 5");
	char * s = erros_get();
	printf("%s", s);
	free(s);
	
}
