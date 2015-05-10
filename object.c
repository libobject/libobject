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
#include "object.h"
#include "alloc.h"
#include "io_helper.h"
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

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

size_t stringLength(const char* source)
{
	if(source == NULL) { return 0; }
	
	return strlen(source);
}

size_t stringHash(const char* source, size_t length)
{
	BUG_ON_NULL(source);

	size_t hash = 0;
	
	for(size_t i = 0; i < length; i++) {
		char c = source[i];
		int a = c - '0';
		hash = (hash * 10) + a;		
	} 

	return hash;
}

static Object*	newObject(ObjectType);
static Map*	newMapInstance(size_t);
static String*	newStringInstance(const char*);
static Array*	newArrayInstance(size_t);
static void	arrayResize(Array*);


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

static Map* newMapInstance(size_t size)
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

Object* newMap(size_t size)
{
	Object* object = newObject(IS_MAP);
	Map*	map    = newMapInstance(size);
	O_MVAL(object) = map;	
	
	return object;
}

/*
 * @return the hashed value
 */
size_t mapInsert(Object* map, const char* key, Object* value)
{
	BUG_ON_NULL(map);
	
	String* keyObject = newStringInstance(key);
	size_t hash = stringHash(keyObject->value, keyObject->length);
	size_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];

	while(bucket != NULL) {
		if((bucket->hash == hash)) {
			bucket->value = value;		
			return hash;
		}
		bucket = bucket->next;
	}

	bucket = ALLOCATE(Bucket);
	BUG_ON_NULL(bucket);
	bucket->key = keyObject;
	bucket->value = value;
	bucket->hash = hash;
	bucket->next = O_MVAL(map)->buckets[bucket_index];	
	O_MVAL(map)->buckets[bucket_index] = bucket;
	O_MVAL(map)->size++;
	
	return hash;
}

size_t mapSize(Object* object)
{
	BUG_ON_NULL(object);
	return O_MVAL(object)->size;
}

size_t mapCapacity(Object* object)
{
	BUG_ON_NULL(object);
	return O_MVAL(object)->capacity;
}

Bucket* mapGetBucket(Object* object, size_t index)
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
	size_t hash = stringHash(key, strlen(key));
	size_t bucket_index = (hash % O_MVAL(map)->capacity);
	Bucket* bucket = O_MVAL(map)->buckets[bucket_index];

	while(bucket != NULL) {
		if((bucket->hash == hash)) {
			return bucket->value;
		}
		bucket = bucket->next;
	}

	return NULL;
}

/*
 * @hash the hashed string value
 */
Object*	mapGetValueByHash(Object* map, size_t hash)
{
	BUG_ON_NULL(map);	
	size_t bucket_index = (hash % O_MVAL(map)->capacity);
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
	size_t length = stringLength(source);
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
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = value;
		O_AVAL(object)->size++;
	} else {
		O_AVAL(object)->table[O_AVAL(object)->nextIndex++] = value;
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

#ifdef O_DEBUG
static void testArray()
{
	Object* array = newArray(4);
	arrayPush(array, newLong(100));
	arrayPush(array, newLong(101));
	arrayPush(array, newString("Fuck"));
	arrayPush(array, newLong(102));
	arrayPush(array, newLong(103));
		
	arrayPush(array, newLong(104));
	arrayPush(array, newLong(105));
	arrayPush(array, newLong(106));
	arrayPush(array, newString("This is a test to test out a string that"));
	arrayPush(array, newString("Fuck This Shit"));
	arrayPush(array, newString("Ryan McCullagh is awesome"));
	arrayPush(array, newString("Ryan McCullagh"));

	Object* innerArray = newArray(8);
	
	arrayPush(innerArray, newLong(LONG_MAX));
	arrayPush(innerArray, newLong(LONG_MAX));
	arrayPush(innerArray, newLong(LONG_MAX));
	arrayPush(innerArray, newLong(LONG_MAX));

	Object* innerInnerArray = newArray(4);
	arrayPush(innerInnerArray, newLong(LONG_MAX));
	arrayPush(innerInnerArray, newLong(LONG_MAX));
	arrayPush(innerInnerArray, newLong(LONG_MAX));
	arrayPush(innerInnerArray, newLong(25));
	arrayPush(innerInnerArray, newDouble(26.444));
	
	arrayPush(array, innerArray);
	arrayPush(innerArray, innerInnerArray);
	
	OBJECT_DUMP(array);

	objectEcho(3, array, innerArray, innerInnerArray);
	PRINT_NL();

}

static void testMap()
{
	Object* map = newMap(8);
	
	mapInsert(map, "firstName", newString("Ryan"));
	mapInsert(map, "lastName", newString("McCullagh"));
	mapInsert(map, "age", newLong(24));

	Object* array = newArray(4);

	arrayMultiPush(array, newString("Programming"), newString("College"), newString("Music"));

	/* WOW this could be bad if it wasn't for that recursion guard */	
	arrayPush(array, array);

	mapInsert(map, "hobbies", array);

	OBJECT_DUMP(map);
}
#endif
#define str(s) #s
#define DEBUG_TRACE(fn) do { \
	fprintf(stdout, "*** BEGIN FUNCTION CALL '%s' ***\n", str(fn)); \
	fn(); \
	fprintf(stdout, "*** END FUNCTION CALL '%s'   ***\n", str(fn)); \
} \
while(0)
