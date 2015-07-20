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

#ifndef OBJECT_H
#define OBJECT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stddef.h>
#include <stdint.h>

/* Forward declaration. See below for definition */
typedef struct Object Object;
typedef struct Object IntegerObject;
typedef struct Object ArrayObject;

typedef enum ObjectType {
	IS_LONG,
	IS_DOUBLE,
	IS_STRING,
	IS_ARRAY,
	IS_MAP,
	IS_OBJECT,
	IS_FUNCTION
} ObjectType;

extern const char *const ObjectPrettyTypeLiteral[];

#define O_PRETTY_TYPE(i) ObjectPrettyTypeLiteral[i]
 
typedef struct String {
	size_t		length;
	char*		value;
} String;

typedef struct Array {
	size_t		capacity;
	size_t		size;
	size_t 		nextIndex;
	Object**	table;
} Array;

typedef struct Bucket {
	String*		key;
	Object*		value;
	int		__notUsed;
	uint32_t	hash;
	struct 		Bucket* next;
} Bucket;

typedef struct Map {
	uint32_t	capacity;
	uint32_t	size;
	Bucket**	buckets;
} Map;

/*
 * 16 byte Object object
 */
struct Object {
	ObjectType	type;
	int		marked;
	union {
		long 		longValue;
		double 		doubleValue;
		String* 	stringValue;
		Array*		arrayValue;
		Map*		mapValue;
		void*		functionValue;
	} value;	
};

#define O_TYPE(o) (o)->type
#define O_MRKD(o) (o)->marked
#define O_LVAL(o) (o)->value.longValue
#define O_DVAL(o) (o)->value.doubleValue
#define O_SVAL(o) (o)->value.stringValue
#define O_AVAL(o) (o)->value.arrayValue
#define O_MVAL(o) (o)->value.mapValue
#define O_FVAL(o) (o)->value.functionValue
/*
 * Scalar public API methods 
 */
Object* 	newLong(long);
Object*		newDouble(double);
/*
 * Public API methods for a Map 
 */
Object*		newMap(uint32_t);
uint32_t	mapInsert(Object*, const char*, Object*);
uint32_t	mapSize(Object*);
uint32_t	mapCapacity(Object*);
Bucket*		mapGetBucket(Object*, uint32_t);
Object*		mapSearch(Object*, const char*);
Object*		mapGetValueByHash(Object*, uint32_t);
void		mapDelete(Object*, const char*);
/*
 * Public API methods for String
 */
Object*		newString(const char*);
/*
 * Public API methods for Function
 */
Object*		newFunction(void*);
/*
 * Public API methods for Array
 */
Object*		newArray(size_t);
void		arrayPush(Object*, Object*);
Object*		arrayGet(Object* object, size_t);
size_t		arraySize(Object*);

uint32_t stringHash(const char* source, size_t length);

#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,N,...) N

/*
 * Public debugging methods
 */
void objectEcho(size_t length, ...);
void objectDump(Object*, Object*, size_t);
void objectDumpEx(Object*, Object*, size_t);

#define OBJECT_DUMP(o) objectDump(o, NULL, 0)

#define OBJECT_DUMP_EX(o) objectDumpEx(o, NULL, 0)

void objectSafeDestroy(Object*, Object*);
#define objectDestroy(o) objectSafeDestroy(o, NULL)

Object* copyObject(Object*);

#define BUG_ON_NULL(o) do { \
	if(o == NULL) { \
		fprintf(stderr, "%s:%s:%d caught a NULL pointer!\n", __FILE__, \
			__FUNCTION__, __LINE__); \
		exit(EXIT_FAILURE); \
	} \
} \
while(0)

#define ARRAY_FOREACH(_ar) do { \
	size_t _i; \
	for(_i = 0; _i < arraySize(_ar); _i++) { \
		Object* _value = arrayGet(_ar, _i); \

#define ARRAY_FOREACH_VALUE(_ar, _val) \
	ARRAY_FOREACH(_ar) \
	_val = _value; \

#define ARRAY_ENUMERATE(_ar, _in, _val) \
	ARRAY_FOREACH(_ar) \
	_in = _i; \
	_val = _value; \

#define ARRAY_FOREACH_END() \
	} \
} while(0)

#define ARRAY_ENUMERATE_END() \
	} \
} while(0)

#define MAP_FOREACH(_ht) do { \
	uint32_t _index; \
	for(_index = 0; _index < mapCapacity(_ht); _index++) { \
		Bucket* _b = (O_MVAL(_ht))->buckets[_index]; \
		if(_b != NULL) { \
			while(_b != NULL) { \
				Bucket* _next = _b->next; \
				Object* _value = copyObject(_b->value); \
				Object* _key = newString(_b->key->value); \

#define MAP_FOREACH_KEY_VALUE(_ht, _k, _val) \
	HT_FOREACH(_ht) \
	_k = _key; \
	_val = _value; \

#define MAP_FOREACH_END() \
				_b = _next; \
			} \
		} \
	} \
} while(0)





























#endif /* OBJECT_H */
