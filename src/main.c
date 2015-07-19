#include <stdio.h>
#include "object.h"

#define HT_FOREACH(_ht) do { \
	uint32_t _index; \
	for(_index = 0; _index < (_ht)->capacity; _index++) { \
		Bucket* _p = (_ht)->buckets[_index]; \
		if(_p != NULL) { \
		Bucket* _pp = _p; \
		while(_pp != NULL) { \
			Object* _value = _pp->value; \

#define HT_FOREACH_VALUE(ht, _val) \
	HT_FOREACH(ht) \
	_val = _value; \
	
#define HT_FOREACH_END() \
				_pp = _pp->next; \
			} \
		} \
	} \
} while(0)

int main(void)
{


	return 0;
}
