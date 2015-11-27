#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_newPair(void)
{

	Object* first = newString("key");
	Object* second = newLong(LONG_MAX);

	Object* pair = newPair(first, second);

	objectDestroy(first);
	objectDestroy(second);
	
	expect(str_equal("key", O_SVAL(O_PVAL(pair)->first)->value));
	expect(O_LVAL(O_PVAL(pair)->second) == LONG_MAX);

	objectDestroy(pair);
}

int main(void)
{

	test_newPair();
}
