#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void test_objectToString(void)
{

	Object* value = newLong(777);

	char* sval = objectToString(value);

	expect(str_equal(sval, "777"));

	free(sval);
	
}

int main(void)
{

	test_objectToString();
}
