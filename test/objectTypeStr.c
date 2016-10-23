#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static const char *const ObjectPrettyTypeLiteral[] = {
	"null",
	"bool",
	"int",
	"float",
	"string",
	"array",
	"map",
	"object",
	"function",
	"pair",
	"pointer"
};

static void test_objectTypeStr(void)
{

	size_t sz = (sizeof(ObjectPrettyTypeLiteral) / sizeof(ObjectPrettyTypeLiteral[0]));
	printf("%zu\n", sz);
	
	Object *s = newString("Ryan McCullagh");
	char* value = objectTypeStr(s);
	expect(value != NULL);
	printf("%s\n", value);
	objectDestroy(s);
	free(value);


}

int main(void)
{

	test_objectTypeStr();
}
