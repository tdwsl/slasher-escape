#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ensure.h"

void ensure(bool cond, const char *desc)
{
	if(cond) return;
	fprintf(stderr, "failed to ____ %s\n", desc);
	exit(1);
}
