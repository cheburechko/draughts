#include "rules.h"
#include "errors.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
	rules_rules * rules;

	rules = rules_get_default();
	rules_destroy(rules);
	if (argc >= 2)
	{
		rules = rules_get(argv[1]);
		char * s = errors_get();
		printf("%s", s);
		free(s);

		rules_destroy(rules);
	}
	return 0;
}
