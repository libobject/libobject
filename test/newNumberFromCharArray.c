#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void test_newNumberFromCharArray(void)
{

	Object* value = newNumberFromCharArray("777");

	expect((O_DVAL(value)) == 777);
	
	objectDestroy(value);
}

int main(void)
{

	test_newNumberFromCharArray();
}
