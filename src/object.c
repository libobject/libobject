/*
 *  Copyright (c) 2015 Ryan McCullagh <me@ryanmccullagh.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "murmurhash3.h"
#include "object.h"

typedef struct MutableString {
        size_t length;
        size_t capacity;
        char* value;
} MutableString;

#define ALLOCATE(t) malloc(sizeof(t))

#define ALLOCATE_TABLE(s, t) calloc(s, sizeof(t))

#define REALLOCATE(p, t, s) realloc(p, (sizeof(t) * s))

#define PRINT_NL() do { \
	fprintf(stdout, "\n"); \
} \
while(0) 

#define ERROR_NO_RETURN(s) do { \
	fprintf(stderr, "%s:%s:%d: %s\n", __FUNCTION__, __FILE__, __LINE__, s); \
	exit(1); \
} \
while(0)

#define RETURN_ON_NULL(o) do { \
	if(o == NULL) { \
		fprintf(stderr, "%s():%s:%d caught a NULL pointer\n", __FILE__, \
			__FUNCTION__, __LINE__); \
		return NULL; \
	} \
} \
while(0)

const char *const ObjectPrettyTypeLiteral[] = {
	"int",
	"float",
	"string",
	"array",
	"map",
	"object",
	"function"
};

uint32_t stringHash(const char* source, size_t length)
{
	BUG_ON_NULL(source);

	uint32_t hash;

	MurmurHash3_x86_32(source, length, 0, &hash);

	return hash;
}

static Object*	newObject(ObjectType);
static Map*	newMapInstance(uint32_t);
static String*	newStringInstance(const char*);
static Array*	newArrayInstance(size_t);
static int	arrayResize(Array*);
static void	mapResize(Map*);
static Object*	arrayRealGet(Array*, size_t);

static Object* newObject(ObjectType type)
{
	Object* object = ALLOCATE(Object);
	RETURN_ON_NULL(object);

	O_TYPE(object) = type;	
	O_MRKD(object) = 0;

	return object;
}

Object* newLong(long value)
{
	Object* object = newObject(IS_LONG);
	O_LVAL(object) = value;

	return object;
}

Object* newDouble(double value)
{
	Object* object = newObject(IS_DOUBLE);
	O_DVAL(object) = value;
	
	return object;
}

static Map* newMapInstance(uint32_t size)
{
	Map* map = ALLOCATE(Map);
	BUG_ON_NULL(map);

	map->capacity = size;
	map->size = 0;
	
	size_t s = (size_t)size;
	Bucket** buckets = ALLOCATE_TABLE(s, Bucket);
	BUG_ON_NULL(buckets);

	map->buckets = buckets;
	
	return map;
}

Object* newMap(uint32_t size)
{
	Object* object = newObject(IS_MAP);
	Map*	map    = newMapInstance(size);
	O_MVAL(object) = map;	
	
	return object;
}

static uint32_t mapIndexFromKey(String* key, uint32_t capacity)
{
	uint32_t hash = stringHash(key->value, key->length);
	return (hash % capacity);
}

Object* copyObject(Object* o)
{
	if(o == NULL) return NULL;
	Object* ret;
	switch(O_TYPE(o)) {
		case IS_LONG: 
			ret = newLong(O_LVAL(o));
		break;
		case IS_DOUBLE:
			ret = newDouble(O_DVAL(o));
		break;
		case IS_STRING: {
			String* str = O_SVAL(o);
			ret = newString(str->value);
		}
		break;
		case IS_ARRAY: {
			size_t i;
			ret = newArray(O_AVAL(o)->capacity);
			for(i = 0; i < O_AVAL(o)->size; i++) {
				Object* value = arrayRealGet(O_AVAL(o), i);
				arrayPush(ret, value);
			}
		}
		break;
		case IS_MAP: {
			uint32_t i;
			ret = newMap(O_MVAL(o)->capacity);
			for(i = 0; i < O_MVAL(o)->capacity; i++) {
				Bucket* b = mapGetBucket(o, i);
				while(b != NULL) {
					Object* value = b->value;
					mapInsert(ret, b->key->value, value);
					b = b->next;
				}
			}
		}
		break;
		default: 
			printf("uh oh\n");
			return NULL;
		break;
	}
	return ret;
}

static int mapTryResize(Map* map)
{
	BUG_ON_NULL(map);
	uint32_t new_capacity = map->capacity * 2;
	size_t n = (size_t)new_capacity;
	Bucket** new_table = ALLOCATE_TABLE(n, Bucket);
	if(new_table == NULL) {
		printf("%s(): failed to allocated %u bytes\n", __func__, new_capacity);
		return 0;
	}
	uint32_t i;
	Bucket** old_table = map->buckets;
	for(i = 0; i < map->capacity; i++) {
		Bucket* b = old_table[i];
		while(b != NULL) {
			Bucket* next = b->next;
			String* key = b->key;
			Object* value = b->value;
			uint32_t new_index = b->hash % new_capacity;
			Bucket* new_bucket = new_table[new_index];
			uint32_t hash = b->hash;
			while(new_bucket != NULL) {
				if((new_bucket->hash == hash) && (key->length == new_bucket->key->length) &&
					(memcmp(new_bucket->key->value, key->value, new_bucket->key->length) == 0)) 
				{
					
					new_bucket->value = value;
				}
				new_bucket = new_bucket->next;
			}
			new_bucket = ALLOCATE(Bucket);
			BUG_ON_NULL(new_bucket);
			new_bucket->key = key;
			new_bucket->value = value;
			new_bucket->hash = hash;
			new_bucket->next = new_table[new_index];
			new_table[new_index] = new_bucket;
			free(b);
			b = next;
		}
	}
	free(old_table);
	map->buckets = new_table;
	map->capacity = new_capacity;
	return 1;
}

static void mapRealDelete(Map* map, String* key, uint32_t hash)
{
	uint32_t index = hash % map->capacity;
	Bucket* b = map->buckets[index];
	Bucket* head = map->buckets[index];
	while(b != NULL) {
		Bucket* next = b->next;
		if((b->hash == hash) && 
			(key->length == b->key->length) &&
			((memcmp(b->key->value, key->value, b->key->length)) == 0))
		{	
			/* first node */
			if(b == map->buckets[index]) { 
				if(b->next == NULL) {
					map->buckets[index] = NULL;
					map->size = map->size-1;
					free(b->key->value);
					free(b->key);
					objectSafeDestroy(b->value, NULL);
					free(b);
					return;
				} else {
					Bucket* bb = b->next;
					if(bb == NULL) {
						printf("bb == NULL\n");
					}
					map->size = map->size-1;
					free(b->key->value);
					free(b->key);
					objectSafeDestroy(b->value, NULL);
					free(b);
					map->buckets[index] = bb;
					return;
				}
			} else {
				/* get the element before the current element */
				while(head->next != NULL && head->next != b) {
					head = head->next;
				}
				head->next = head->next->next;
				map->buckets[index] = head;
				map->size = map->size-1;
				free(b->key->value);
				free(b->key);
				objectSafeDestroy(b->value, NULL);
				free(b);
				return;
			}
		}
		b = next;
	}
}

void mapDelete(Object* object, const char* pkey)
{
	BUG_ON_NULL(object);
	BUG_ON_NULL(pkey);
	if(O_TYPE(object) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return;
	}

	String* key = newStringInstance(pkey);
	uint32_t hash = stringHash(key->value, key->length);
	mapRealDelete(O_MVAL(object), key, hash);
	free(key->value);
	free(key);
}


/*
 * @return the hashed value
 */
uint32_t mapInsert(Object* map, const char* key, Object* value)
{
	BUG_ON_NULL(map);
	if(O_TYPE(map) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return 0;
	}
	if(O_MVAL(map)->size >= O_MVAL(map)->capacity) {
		int status;
		if((status = mapTryResize(O_MVAL(map))) == 0) {
			printf("%s(): failed to resize table\n", __func__);
			return 0;		
		} else {
			#ifdef DEBUG
			printf("%s(): resizing table\n", __func__);
			#endif
		}
	}
	
	Object* value_copy = copyObject(value);
	if(value_copy == NULL) {
		printf("%s(): failed to copy object\n", __func__);
		return 0;
	}

	String* keyObject = newStringInstance(key);
	uint32_t hash = stringHash(keyObject->value, keyObject->length);
	uint32_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];
	while(bucket != NULL) {
		if((bucket->hash == hash) && 
			(keyObject->length == bucket->key->length) &&
			((memcmp(bucket->key->value, keyObject->value, bucket->key->length)) == 0))
		{
			Object* oldValue = bucket->value;
			/* free the old value */
			objectSafeDestroy(oldValue, NULL);
			bucket->value = value_copy;		
			/* not used if it exists already */
			free(keyObject->value);
			free(keyObject);
			return hash;
		}
		bucket = bucket->next;
	}

	bucket = ALLOCATE(Bucket);
	BUG_ON_NULL(bucket);
	bucket->key = keyObject;
	bucket->value = value_copy;
	bucket->hash = hash;
	bucket->next = O_MVAL(map)->buckets[bucket_index];	
	O_MVAL(map)->buckets[bucket_index] = bucket;
	O_MVAL(map)->size++;
	
	return hash;
}

uint32_t mapSize(Object* object)
{
	BUG_ON_NULL(object);
	return O_MVAL(object)->size;
}

uint32_t mapCapacity(Object* object)
{
	BUG_ON_NULL(object);
	if(O_TYPE(object) != IS_MAP) {
		return 0;
	}
	return O_MVAL(object)->capacity;
}

Bucket* mapGetBucket(Object* object, uint32_t index)
{
	BUG_ON_NULL(object);
	Map* map = O_MVAL(object);
	
	if(index >= map->capacity) {
		ERROR_NO_RETURN("RangeError: index of out of range");
	}
	
	return map->buckets[index];
}
/*
 * search the Map for a value with the key equal to key.
 * return a COPY of the value if found, NULL if not
 * the caller is resposible for freeing the returned value
 */
Object*	mapSearch(Object* map, const char* key)
{
	BUG_ON_NULL(map);
	size_t key_length = strlen(key);	
	uint32_t hash = stringHash(key, key_length);
	uint32_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];

	while(bucket != NULL) {
		if((bucket->hash == hash) && 
			(key_length == bucket->key->length) &&
			((memcmp(bucket->key->value, key, bucket->key->length)) == 0))
		{
			return copyObject(bucket->value);
		}

		bucket = bucket->next;
	}

	return NULL;
}

/*
 * @hash the hashed string value
 * this doesn't work if two keys hash to the same value
 */
Object*	mapGetValueByHash(Object* map, uint32_t hash)
{
	BUG_ON_NULL(map);	
	uint32_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];
	
	while(bucket != NULL) {
		if((bucket->hash == hash)) {
			return bucket->value;
		}
		bucket = bucket->next;
	}

	return NULL;
}
static String* newStringInstance(const char* source)
{
	BUG_ON_NULL(source);	
	String* string = ALLOCATE(String);
	BUG_ON_NULL(string);
	size_t length = strlen(source);
	string->length = length;
	string->value = malloc(sizeof(char) * length + 1);
	BUG_ON_NULL(string->value);
	size_t i;
	for(i = 0; i < length; i++) {
		string->value[i] = source[i];
	}

	string->value[length] = '\0';
	
	return string;
}

Object* newString(const char* value)
{
	BUG_ON_NULL(value);
	
	Object* object = newObject(IS_STRING);
	String* string = newStringInstance(value);

	O_SVAL(object) = string;	
	
	return object;
}

Object* newFunction(void* ptr)
{
	BUG_ON_NULL(ptr);
	Object* object = newObject(IS_FUNCTION);

	O_FVAL(object) = ptr;
	
	return object;
}

static Array* newArrayInstance(size_t size)
{
	Array* array = ALLOCATE(Array);
	RETURN_ON_NULL(array);
	
	Object** table = ALLOCATE_TABLE(size, Object);
	if(table == NULL) {
		free(array);
		printf("%s(): failed to allocate %zu bytes for array->table\n", __func__, size);
		return NULL;
	}

	array->capacity = size;
	array->size = 0;
	array->nextIndex = 0;
	array->table = table;

	return array;
}

Object* newArray(size_t size)
{
	Object* object = newObject(IS_ARRAY);
	if(object == NULL) {
		printf("%s(): newObject() returned NULL\n", __func__);
		return NULL;
	}
	
	Array* array = newArrayInstance(size);
	if(array == NULL) {
		free(object);
		printf("%s(): newArrayInstance() returned NULL\n", __func__);
		return NULL;
	}
	O_AVAL(object) = array;
	
	return object;
}

static int arrayResize(Array* array)
{
	if(array == NULL) {
		printf("%s(): passing NULL pointer not allowed\n", __func__);
		return 0;
	}

	size_t new_capacity = array->capacity * 2;
	
	Object** new_table = REALLOCATE(array->table, Object, new_capacity);
      	if(new_table == NULL) {
		printf("%s(): failed to reallocate %zu bytes\n", __func__, new_capacity);
		return 0;
	}	

	array->capacity = new_capacity;
	array->table = new_table;
	return 1;
}

void arrayPush(Object* object, Object* value)
{
	BUG_ON_NULL(object);
	
	if(O_TYPE(object) != IS_ARRAY) {
		printf("%s(): Object type is not an instance of Array, got %d\n", __func__, O_TYPE(object));
		return;		
	}

	Object* value_copy = copyObject(value);
	if(value_copy == NULL) {
		printf("%s(): failed to push value, copyObject returned NULL\n", __func__);
		return;
	}

	if(O_AVAL(object)->capacity == O_AVAL(object)->size) {
		if(!arrayResize(O_AVAL(object))) {
			objectDestroy(value_copy);
			return;
		}
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = value_copy;
		O_AVAL(object)->size++;
	} else {
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = value_copy;
		O_AVAL(object)->size++;
	}
}
/*
 * return a pointer to value at index i
 */
static Object* arrayRealGet(Array* array, size_t index)
{
	return array->table[index];
}

Object* arrayGet(Object* object, size_t index)
{
	BUG_ON_NULL(object);

	if(O_TYPE(object) != IS_ARRAY) {
		printf("%s(): Object type is not an instance of Array, got %d\n", __func__, O_TYPE(object));
		return NULL;		
	}

	if(index < O_AVAL(object)->size) {
		return copyObject(arrayRealGet(O_AVAL(object), index));
	}
	
	return NULL;
}

size_t arraySize(Object* object)
{
	BUG_ON_NULL(object);
	
	if(O_TYPE(object) != IS_ARRAY) {
		printf("%s(): Object type is not an instance of Array, got %d\n", __func__, O_TYPE(object));
		return 0;		
	}
	
	return O_AVAL(object)->size;
}

void objectEcho(size_t length, ...)
{
	va_list args;
	va_start(args, length);
	size_t i;
	for(i = 0; i < length; i++) {
		
		Object* object = va_arg(args, Object*);
		
		BUG_ON_NULL(object);

		switch(O_TYPE(object)) {
			case IS_LONG:
				fprintf(stdout, "%ld", O_LVAL(object));
			break;
			case IS_DOUBLE:
				fprintf(stdout, "%.2f", O_DVAL(object));
			break;
			case IS_STRING:
				fprintf(stdout, "%s", O_SVAL(object)->value);
			break;
			case IS_ARRAY:
				fprintf(stdout, "[Object Array]");
			break;
			case IS_MAP:
				fprintf(stdout, "[Object Map]");
			break;
			default:
				fprintf(stdout, "[Object Object]");
			break;
		}

		fprintf(stdout, " ");
	}
		
	va_end(args);
}

#define INDENT_LOOP(c) do { \
	size_t i; \
	for(i = 0; i < c; i++) { \
		fprintf(stdout, "\t"); \
	} \
} \
while(0)

void objectDump(Object* object, Object* last, size_t indent)
{
	BUG_ON_NULL(object);
	size_t i;
	
	switch(O_TYPE(object)) {
		case IS_LONG:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_LONG));
			fprintf(stdout, "(");
			fprintf(stdout, "%ld", O_LVAL(object));
			fprintf(stdout, ")");
			fprintf(stdout, "\n");
		break; 
		case IS_DOUBLE:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_DOUBLE));
			fprintf(stdout, "(");
			fprintf(stdout, "%.2f", O_DVAL(object));
			fprintf(stdout, ")");
			fprintf(stdout, "\n");
		break; 
		case IS_STRING:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_STRING));
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", O_SVAL(object)->length);
			fprintf(stdout, ")");
			fprintf(stdout, " ");
			fprintf(stdout, "\"%s\"", O_SVAL(object)->value);
			fprintf(stdout, "\n");

		break;
		case IS_ARRAY:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_ARRAY));
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", O_AVAL(object)->size);
			fprintf(stdout, ")");
			fprintf(stdout, " {");
			fprintf(stdout, "\n");
			for(i = 0; i < arraySize(object); i++) {
				if(object == last) {
					INDENT_LOOP(indent);
					fprintf(stdout, "\t[Circular]\n");
					INDENT_LOOP(indent);
					fprintf(stdout, "}");
					fprintf(stdout, "\n");
					return;
				} else {
					INDENT_LOOP(indent);
					fprintf(stdout, "\t[%zu] => ", i);
					objectDump(arrayRealGet(O_AVAL(object), i), object,
						indent + 1);
				}
			}
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		break;
		case IS_MAP: {
			uint32_t i;
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_MAP));
			fprintf(stdout, "(");
			fprintf(stdout, "%u", mapSize(object));
			fprintf(stdout, ")");
			fprintf(stdout, " {");
			fprintf(stdout, "\n");
			for(i = 0; i < mapCapacity(object); i++) {
				Bucket* b = mapGetBucket(object, i);
				if(b != NULL) {
					Bucket* bb = b;
					while(bb != NULL) {	
						fprintf(stdout, "\t");
						fprintf(stdout, "%s", 
							bb->key->value);
						fprintf(stdout, ": ");	
				
						if(object == last) {
							fprintf(stdout,
							"**RECURSION**\n");
							INDENT_LOOP(indent);
							fprintf(stdout, "}");
							fprintf(stdout, "\n");
							return;
						}
						objectDump(bb->value, object,
							indent + 1);	
						bb = bb->next;
					}
				}		
			}		
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		}
		break;
		default:
			fprintf(stdout, "[Object <none>]\n");
		break;
	}
}


void objectDumpEx(Object* object, Object* last, size_t indent)
{
	BUG_ON_NULL(object);
	size_t i;
	
	switch(O_TYPE(object)) {
		case IS_LONG:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_LONG));
			fprintf(stdout, "(");
			fprintf(stdout, "%p", (void *)object);
			fprintf(stdout, ")");
			fprintf(stdout, "\n");
		break; 
		case IS_STRING:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_STRING));
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", O_SVAL(object)->length);
			fprintf(stdout, ")");
			fprintf(stdout, " ");
			fprintf(stdout, "%p", (void *)O_SVAL(object)->value);
			fprintf(stdout, "\n");

		break;
		case IS_ARRAY:
			fprintf(stdout, "%s => %p", O_PRETTY_TYPE(IS_ARRAY), (void *)object);
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", O_AVAL(object)->size);
			fprintf(stdout, ")");
			fprintf(stdout, " {");
			fprintf(stdout, "\n");
			for(i = 0; i < arraySize(object); i++) {
				if(object == last) {
					INDENT_LOOP(indent);
					fprintf(stdout, "\t[Circular]\n");
					INDENT_LOOP(indent);
					fprintf(stdout, "}");
					fprintf(stdout, "\n");
					return;
				} else {
					INDENT_LOOP(indent);
					fprintf(stdout, "\t[%zu] => ", i);
					objectDumpEx(arrayRealGet(O_AVAL(object), i), object,
						indent + 1);
				}
			}
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		break;
		case IS_MAP: {
			fprintf(stdout, "%s => %p", O_PRETTY_TYPE(IS_MAP), (void *)object);
			fprintf(stdout, "(");
			fprintf(stdout, "%u", mapSize(object));
			fprintf(stdout, ")");
			fprintf(stdout, " {");
			fprintf(stdout, "\n");
			uint32_t ii;
			for(ii = 0; ii < mapCapacity(object); ii++) {
				Bucket* b = mapGetBucket(object, ii);
				if(b != NULL) {
					Bucket* bb = b;
					while(bb != NULL) {	
						fprintf(stdout, "\t");
						fprintf(stdout, "%s", 
							bb->key->value);
						fprintf(stdout, ": ");	
				
						if(object == last) {
							fprintf(stdout,
							"**RECURSION**\n");
							INDENT_LOOP(indent);
							fprintf(stdout, "}");
							fprintf(stdout, "\n");
							return;
						}
						objectDumpEx(bb->value, object,
							indent + 1);	
						bb = bb->next;
					}
				}		
			}		
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		}
		break;
		default:
			fprintf(stdout, "[Object <none>]\n");
		break;
	}
}

void objectSafeDestroy(Object* current, Object* last)
{
	if(current == NULL) {
		printf("current == NULL\n");
		return;
	}
	switch(O_TYPE(current)) {
		case IS_LONG:
		case IS_DOUBLE:
			free(current);
		break;
		case IS_STRING:
			free(O_SVAL(current)->value);
			free(O_SVAL(current));
			free(current);
		break;
		case IS_ARRAY: {
			size_t i;
			for(i = 0; i < arraySize(current); i++) {
				if(current == last) {
					printf("circular\n");
					return;
				} else {
					Object* value = arrayRealGet(O_AVAL(current), i);
					objectSafeDestroy(value, current);
				}
			}
			free(O_AVAL(current)->table);
			free(O_AVAL(current));
			free(current);
		}
		break;
		case IS_MAP: {
			uint32_t i;
			for(i = 0; i < mapCapacity(current); i++) {
				Bucket* b = mapGetBucket(current, i);
				while(b != NULL) {
					Bucket* bn = b->next;
					String* key = b->key;
					free(key->value);
					free(key);
					objectSafeDestroy(b->value, current);
					free(b);
					b = bn;
				}
			}
			free(O_MVAL(current)->buckets);
			free(O_MVAL(current));
			free(current);
		}
		break;
		default:
			printf("%s(): invalid object\n", __func__);
			return;
		break;
	}
}
static MutableString* newMutableString()
{
        MutableString* ms;
        if((ms = malloc(sizeof(MutableString))) == NULL) {
                return NULL;
        }
        ms->length = 0;
        ms->capacity = 16;
        if((ms->value = malloc(ms->capacity)) == NULL) {
                free(ms);
                return NULL;
        }
        ms->value[ms->length] = '\0';
        return ms;
}

static void mutableStringAppend(MutableString* ms, const char value)
{
        size_t part_length = 1;
        size_t i;
        size_t current_position;
        size_t new_length = part_length + ms->length;
        current_position = ms->length;

        if(new_length >= ms->capacity) {
                while(new_length >= ms->capacity) {
                        ms->capacity *=2;
                        if((ms->value = realloc(ms->value, ms->capacity)) == NULL) {
                                return;
                        }
                }
        }
        for(i = 0; i < part_length; i++) {
                ms->value[current_position++] = value;
        }
        ms->length = new_length;
        ms->value[current_position] = '\0';
}

static void mutableStringReset(MutableString* ms)
{
        ms->length = 0;
        memset(ms->value, '\0', ms->capacity);
}

static void mutableStringFree(MutableString* ms)
{
        free(ms->value);
        free(ms);
}

Object* stringSplit(const char* source, char sep)
{
        BUG_ON_NULL(source);
        size_t source_len = strlen(source);
        if(!source_len)
                return newArray(2);

        MutableString* key = newMutableString();
        Object* array = newArray(2);
        size_t i;
        for(i = 0; i < source_len; i++) {
                if(source[i] == sep) {
                        if(i != 0) {
                                arrayPush(array, newString(key->value));
                                mutableStringReset(key);
                        }
                        continue;
                }
                mutableStringAppend(key, source[i]);
        }
        arrayPush(array, newString(key->value));
        mutableStringFree(key);
        return array;
}

