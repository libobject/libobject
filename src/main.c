#include <stdio.h>
#include "object.h"

int main(void)
{

	Object* map = newMap(2);
	mapInsert(map, "name", newString("Ryan McCullagh"));
	mapInsert(map, "test1", newString("Ryan"));

	OBJECT_DUMP(map);	
	mapInsert(map, "test", newString("Ryan"));
	
	OBJECT_DUMP(map);	
	Object* found = mapSearch(map, "name");
	if(found != NULL) {
		printf("Found name.\n");
		OBJECT_DUMP(found);
	} else {
		printf("name not found\n");
	}	

	// M/N = loadFactor; m = size, n = capacity
	// maintain a .5 load factor	
	float loadFactor = ((float)((O_MVAL(map)->size)) / ((float)(O_MVAL(map)->capacity)));

	printf("%f\n", loadFactor);

	return 0;
}
