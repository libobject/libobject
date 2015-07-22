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
static char*	toString(Object*);
static int	stringRealCompare(String*, String*);
static Bucket*	bucketRealSearch(Bucket*, String*);
static Object*	mapRealSearch(Map*, String*);
static String*	stringCopy(String*);

static char* toString(Object* this)
{
	BUG_ON_NULL(this);
	char* buffer = NULL;
	switch(O_TYPE(this)) {
		case IS_LONG: {
			buffer = malloc(32);
			BUG_ON_NULL(buffer);
			int ret = snprintf(buffer, 32, "%ld", O_LVAL(this));	
		}
		break;
		case IS_DOUBLE: {
			buffer = malloc(32);
			BUG_ON_NULL(buffer);
			int ret = snprintf(buffer, 32, "%f", O_DVAL(this));
		}
		break;
		case IS_STRING: {
			buffer = malloc(O_SVAL(this)->length + 1);	
			BUG_ON_NULL(buffer);
			memcpy(buffer, O_SVAL(this)->value, O_SVAL(this)->length);
			buffer[O_SVAL(this)->length] = '\0';				
		}
		break;
		case IS_ARRAY: {
			const char* array = "[object Array]";
			size_t length = sizeof("[object Array]") -1;	
			buffer = malloc(length + 1);
			BUG_ON_NULL(buffer);
			memcpy(buffer, array, length);
			buffer[length] = '\0';
		}
		break;
		case IS_MAP: {
			const char* map = "[object Map]";
			size_t length = sizeof("[object Map]") -1;	
			buffer = malloc(length + 1);
			BUG_ON_NULL(buffer);
			memcpy(buffer, map, length);
			buffer[length] = '\0';
		}
		break;
		case IS_OBJECT:
		case IS_FUNCTION: {
			const char* object = "[object Object]";
			size_t length = sizeof("[object Object]") -1;
			buffer = malloc(length + 1);
			BUG_ON_NULL(buffer);
			memcpy(buffer, object, length);
			buffer[length] = '\0';
		}
		break;
	}

	return buffer;
}

static Object* newObject(ObjectType type)
{
	Object* object = ALLOCATE(Object);
	RETURN_ON_NULL(object);

	object->toString = toString;
	O_TYPE(object) = type;	
	O_MRKD(object) = 0;

	return object;
}

Object* newLong(long value)
{
	Object* object = newObject(IS_LONG);
	RETURN_ON_NULL(object);
	O_LVAL(object) = value;

	return object;
}

Object* newDouble(double value)
{
	Object* object = newObject(IS_DOUBLE);
	RETURN_ON_NULL(object);
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
	map->first = NULL;
	map->last = NULL;	

	return map;
}

Object* newMap(uint32_t size)
{
	Object* object = newObject(IS_MAP);
	RETURN_ON_NULL(object);
	Map*	map    = newMapInstance(size);
	RETURN_ON_NULL(map);	
	O_MVAL(object) = map;	
	return object;
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
			Bucket* p = O_MVAL(o)->first;
			while(p != NULL) {
				Object* value = p->value;
				/* mapInsert calls copyObject */
				mapInsert(ret, p->key->value, value);
				p = p->iNext;
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
		if(b) {
			uint32_t new_index = b->hash % new_capacity;
			new_table[new_index] = b;
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

static uint32_t mapRealInsert(Map* map, String* key, Object* value)
{
	uint32_t i = key->hash % map->capacity;
	Bucket* position = map->buckets[i];
	Bucket* found = bucketRealSearch(position, key);
	
	if(found != NULL) {
		Object* oldValue = found->value;
		objectSafeDestroy(oldValue, NULL);
		found->value = value;	
		return found->key->hash;
	}
		
	position = calloc(1, sizeof(Bucket));
	BUG_ON_NULL(position);
	String* key_copy = stringCopy(key);
	BUG_ON_NULL(key_copy);
	position->key = key_copy;
	position->value = value;
	position->hash = key->hash;
	position->next = map->buckets[i];
	
	if(map->last == NULL) {
		map->first = position;
		map->last = position;
	} else {
		map->last->iNext = position;
		position->iPrev = map->last;
		map->last = position;
	}
	
	map->buckets[i] = position;
	map->size++;

	return key->hash;
}

/*
 * @return the hashed value
 */
uint32_t mapInsert(Object* map, const char* key, Object* value)
{
	BUG_ON_NULL(map);
	BUG_ON_NULL(key);
	BUG_ON_NULL(value);
	if(O_TYPE(map) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return 0;
	}
	if(O_MVAL(map)->size >= O_MVAL(map)->capacity) {
		if(!mapTryResize(O_MVAL(map))) {
			#ifdef DEBUG
			printf("%s(): failed to resize table\n", __func__);
			#endif
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
	uint32_t hash = mapRealInsert(O_MVAL(map), keyObject, value_copy);
	free(keyObject->value);
	free(keyObject);
	return hash;	
}


uint32_t mapSize(Object* object)
{
	BUG_ON_NULL(object);
	if(O_TYPE(object) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return 0;
	}
	return O_MVAL(object)->size;
}

uint32_t mapCapacity(Object* object)
{
	BUG_ON_NULL(object);
	if(O_TYPE(object) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return 0;
	}
	return O_MVAL(object)->capacity;
}

Bucket* mapGetBucket(Object* object, uint32_t index)
{
	BUG_ON_NULL(object);
	if(O_TYPE(object) != IS_MAP) {
		printf("%s(): Object passed must be an instance of Map\n", __func__);
		return NULL;
	}
	Map* map = O_MVAL(object);
	
	if(index >= map->capacity) {
		ERROR_NO_RETURN("RangeError: index of out of range");
	}
	
	return map->buckets[index];
}

static String* stringCopy(String* value)
{
	BUG_ON_NULL(value);
	String* str = calloc(1, sizeof(String));
	BUG_ON_NULL(str);
	str->hash = value->hash;
	str->length = value->length;
	str->value = malloc(value->length + 1);
	BUG_ON_NULL(str->value);
	memcpy(str->value, value->value, value->length);
	str->value[value->length] = '\0';	
	return str;
}

static int stringRealCompare(String* left, String* right)
{
	if((left->hash == right->hash) &&
		(left->length == right->length) &&
		((memcmp(left->value, right->value, left->length)) == 0)) {
		return 1;
	}
	return 0;
}

static Bucket* bucketRealSearch(Bucket* bucket, String* key)
{
	while(bucket != NULL) {
		/* if there is only 1 item in bucket, then don't do memcmp */
		if(stringRealCompare(bucket->key, key)) {
			return bucket;
		}
		bucket = bucket->next;
	}
	return bucket;	
}

static Object* mapRealSearch(Map* map, String* key)
{
	Bucket* bucket;
	bucket = bucketRealSearch(map->buckets[key->hash % map->capacity], key);		
	if(bucket != NULL) {
		return bucket->value;
	} 
	return NULL;
}

/*
 * search the Map for a value with the key equal to key.
 * return a COPY of the value if found, NULL if not
 * the caller is resposible for freeing the returned value
 * we traverse the list out of order of insertion
 */
Object*	mapSearch(Object* map, const char* pkey)
{
	BUG_ON_NULL(map);
	BUG_ON_NULL(pkey);
	
	if(O_TYPE(map) != IS_MAP)
		return NULL;		

	String* key = newStringInstance(pkey);
	if(!key) 
		return NULL;
	
	Object* ret = copyObject(mapRealSearch(O_MVAL(map), key));
	free(key->value);
	free(key);
	return ret;
}



static String* newStringInstance(const char* source)
{
	BUG_ON_NULL(source);	
	String* string = ALLOCATE(String);
	BUG_ON_NULL(string);
	size_t length = strlen(source);
	string->length = length;
	string->value = malloc(sizeof(char) * length + 1);
	string->hash = stringHash(source, length);
	string->_notUsed = 0;	
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

static int arrayRealTryResize(Array* array)
{

	return 1;
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
			Bucket* p = O_MVAL(object)->first;
			while(p != NULL) {	
				fprintf(stdout, "\t");
				fprintf(stdout, "%s", 
					p->key->value);
				fprintf(stdout, ": ");	
		
				if(object == last) {
					fprintf(stdout,
					"**RECURSION**\n");
					INDENT_LOOP(indent);
					fprintf(stdout, "}");
					fprintf(stdout, "\n");
					return;
				}
				objectDump(p->value, object,
					indent + 1);	
				p = p->iNext;
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

void objectDestruct(void* o)
{
	objectSafeDestroy((Object*)(o), NULL);
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
			printf("%s(): invalid object:%p, type:%d\n", __func__, (void*)(current), O_TYPE(current));
			return;
		break;
	}
}
