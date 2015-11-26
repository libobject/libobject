#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"


static void test_objectToJson(void)
{
	Object* value = newArray(2);

	Object* item = newLong(1);
	arrayPush(value, item);

	objectDestroy(item);

	size_t length = 0;

	char *jval = objectToJson(value, 0, &length);
	
	expect(str_equal(jval, "[1]"));
	
	free(jval);
	
	objectDestroy(value);
}

int main(void)
{

	test_objectToJson();
}
