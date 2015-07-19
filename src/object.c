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

#define ALLOCATE(t) malloc(sizeof(t))

#define ALLOCATE_TABLE(s, t) calloc(s, sizeof(t))

#define REALLOCATE(p, t, s) realloc(p, (sizeof(t) * s))

#define BUG_ON_NULL(o) do { \
	if(o == NULL) { \
		fprintf(stderr, "%s:%s:%d caught a NULL pointer!\n", __FILE__, \
			__FUNCTION__, __LINE__); \
		exit(EXIT_FAILURE); \
	} \
} \
while(0)

#define PRINT_NL() do { \
	fprintf(stdout, "\n"); \
} \
while(0) 

#define ERROR_NO_RETURN(s) do { \
	fprintf(stderr, "%s:%s:%d: %s\n", __FUNCTION__, __FILE__, __LINE__, s); \
	exit(1); \
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
static void	arrayResize(Array*);
static void	mapResize(Map*);

static Object* newObject(ObjectType type)
{
	Object* object = ALLOCATE(Object);
	BUG_ON_NULL(object);
	
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
	if(size <= 1) {
		ERROR_NO_RETURN("size must be greater than 1");
	}

	Map* map = ALLOCATE(Map);
	BUG_ON_NULL(map);

	map->capacity = size;
	map->size = 0;
	
	Bucket** buckets = ALLOCATE_TABLE(size, Bucket);
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
				Object* value = arrayGet(o, i);
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

static void mapSafeResize(Map* map)
{
	Bucket** old_table = map->buckets;
	uint32_t new_capacity = map->capacity * 2;
	Bucket** new_table = ALLOCATE_TABLE(new_capacity, Bucket);
	BUG_ON_NULL(new_table);
	uint32_t i;
	for(i = 0; i < map->capacity; i++) {
		Bucket* b = old_table[i];
		while(b != NULL) {
			Bucket* bnext = b->next;
			String* key = b->key;
			uint32_t new_index = mapIndexFromKey(key, new_capacity);
			b->next = new_table[new_index];
			new_table[new_index] = b;
			b = bnext;
		}

	}
	free(old_table);
	map->capacity = new_capacity;
	map->buckets = new_table;
}

static void mapResize(Map* map)
{
	uint32_t newCapacity = map->capacity * 2;
	uint32_t newSize     = 0;
	Bucket** b = ALLOCATE_TABLE(newCapacity, Bucket);
	BUG_ON_NULL(b);
	uint32_t i;
	for(i = 0; i < map->capacity; i++) {
		Bucket* bucket = map->buckets[i];
		if(bucket != NULL) {
			while(bucket != NULL) {
				String* key   = bucket->key;
				Object* value = bucket->value;
				uint32_t hash = stringHash(key->value, key->length);
				uint32_t index = (hash % newCapacity);
				Bucket* bb = b[index];
				while(bb != NULL) {
					if((bb->hash == hash) && 
					 (key->length == bb->key->length) &&
					 (memcmp(bb->key->value, key->value, bb->key->length) == 0)) {
						bb->value = value;

					}
					bb = bb->next;
				}

				bb = ALLOCATE(Bucket);
				BUG_ON_NULL(bb);
				bb->key = key;
				bb->value = value;
				bb->hash = hash;
				bb->next = b[index];	
				b[index] = bb;	
				newSize++;
				bucket = bucket->next;
			}
		}
	}
	map->buckets = b;
	map->size = newSize;
	map->capacity = newCapacity;	
}

/*
 * @return the hashed value
 */
uint32_t mapInsert(Object* map, const char* key, Object* value)
{
	BUG_ON_NULL(map);
	if(O_MVAL(map)->capacity == O_MVAL(map)->size) {
		mapSafeResize(O_MVAL(map));
	}

	String* keyObject = newStringInstance(key);
	uint32_t hash = stringHash(keyObject->value, keyObject->length);
	uint32_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];
	Object* value_copy = copyObject(value);
	if(value_copy == NULL) {
		printf("value_copy == NULL\n");
	}
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
			return bucket->value;
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

	for(size_t i = 0; i < length; i++) {
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
	if(size <= 1) {
		ERROR_NO_RETURN("size must be greater than 1");
	}
	
	Array* array = ALLOCATE(Array);
	BUG_ON_NULL(array);
	
	Object** table = ALLOCATE_TABLE(size, Object);
	BUG_ON_NULL(table);
	
	array->capacity = size;
	array->size = 0;
	array->nextIndex = 0;
	array->table = table;

	return array;
}

static void arrayResize(Array* array)
{
        BUG_ON_NULL(array);

	size_t newSize = array->capacity *= 2;

        array->capacity = newSize;
	array->table = REALLOCATE(array->table, Object, newSize);
	
	BUG_ON_NULL(array->table);
}

Object* newArray(size_t size)
{
	Object* object = newObject(IS_ARRAY);
	Array* array = newArrayInstance(size);
	O_AVAL(object) = array;
	
	return object;
}

void arrayPush(Object* object, Object* value)
{
	BUG_ON_NULL(object);

	if(O_AVAL(object)->capacity == O_AVAL(object)->size) {
		arrayResize(O_AVAL(object));
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = copyObject(value);
		O_AVAL(object)->size++;
	} else {
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = copyObject(value);
		O_AVAL(object)->size++;
	}
}

size_t arrayPop(Object* object, Object** ret)
{
	if(O_AVAL(object)->nextIndex == 0) {
		*ret = NULL;
		return 0;
	} else {
		O_AVAL(object)->nextIndex--;	
		*ret = O_AVAL(object)->table[O_AVAL(object)->nextIndex];
		return O_AVAL(object)->nextIndex;
	}
}

Object* arrayGet(Object* object, size_t index)
{
	BUG_ON_NULL(object);

	if(index < O_AVAL(object)->capacity) {
		return O_AVAL(object)->table[index];
	}
	
	return NULL;
}

size_t arraySize(Object* object)
{
	BUG_ON_NULL(object);
	return O_AVAL(object)->size;
}

void __arrayMultiPush(Object* object, size_t length, ...)
{
	BUG_ON_NULL(object);
	va_list args;
	va_start(args, length);
	
	for(size_t i = 0; i < length; i++) {
		Object* value = va_arg(args, Object*);
		BUG_ON_NULL(value);
		arrayPush(object, value);
	}

	va_end(args);
}

void objectEcho(size_t length, ...)
{
	va_list args;
	va_start(args, length);
	
	for(size_t i = 0; i < length; i++) {
		
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
	for(size_t i = 0; i < c; i++) { \
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
					objectDump(arrayGet(object, i), object,
						indent + 1);
				}
			}
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		break;
		case IS_MAP:
			fprintf(stdout, "%s", O_PRETTY_TYPE(IS_MAP));
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", mapSize(object));
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
					objectDumpEx(arrayGet(object, i), object,
						indent + 1);
				}
			}
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
		break;
		case IS_MAP:
			fprintf(stdout, "%s => %p", O_PRETTY_TYPE(IS_MAP), (void *)object);
			fprintf(stdout, "(");
			fprintf(stdout, "%zu", mapSize(object));
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
						objectDumpEx(bb->value, object,
							indent + 1);	
						bb = bb->next;
					}
				}		
			}		
			INDENT_LOOP(indent);
			fprintf(stdout, "}");
			fprintf(stdout, "\n");
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
					Object* value = arrayGet(current, i);
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
