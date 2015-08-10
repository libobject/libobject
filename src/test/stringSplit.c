#include "../object.h"
#include <stdlib.h>
#include <stdio.h>

#define OBJECT_ASSERT(expr, f) do { \
        if(!expr) { \
                fprintf(stderr, "%s(): Assertion failed @ line %d\n", f, __LINE__); \
                exit(1); \
        } \
} while(0);

#define OBJECT_TEST

#define OBJECT_TEST_PASS(f) do { \
        fprintf(stderr, "%s(): Passed!\n", f); \
} while(0);

OBJECT_TEST void Assert_stringSplit_Returns_Empty_Array_When_First_Argument_Is_Length_0()
{
        
	OBJECT_ASSERT((arraySize(stringSplit("", ' ')) == 0), "stringSplit");
	OBJECT_TEST_PASS("stringSplit");
}

int main()
{
        Assert_stringSplit_Returns_Empty_Array_When_First_Argument_Is_Length_0();

        return 0;
}

