#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void test_stringCat(void)
{

	Object* first = newString("Ryan");
	Object* last = newString(" McCullagh");
	
	Object* full = stringCat(first, last);

	expect(O_SVAL(full)->length == 14);
 
	fprintf(stdout, "%s\n", O_SVAL(full)->value);

	objectDestroy(first);
	objectDestroy(last);
	objectDestroy(full);

	
}

int main(void)
{

	test_stringCat();
}
