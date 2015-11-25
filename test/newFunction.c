#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_func(void)
{
	return;
}

static void test_newFunction(void)
{

	Object* value = newFunction(test_func);

	expect(test_func == (void(*)(void))O_FVAL(value));
	
	objectDestroy(value);
}

int main(void)
{

	test_newFunction();
}
