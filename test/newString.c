#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void test_newString(void)
{

	Object* value = newString("Ryan McCullagh");

	expect(str_equal(O_SVAL(value)->value, "Ryan McCullagh"));
	
	objectDestroy(value);
}

int main(void)
{

	test_newString();
}
