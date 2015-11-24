#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_newDouble(void)
{

	Object* value = newDouble(LONG_MAX);

	expect(O_DVAL(value) == LONG_MAX);
	
	objectDestroy(value);
}

int main(void)
{

	test_newDouble();
}
