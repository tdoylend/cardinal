#ifndef CARDINAL_LIST_H
#define CARDINAL_LIST_H

#include <cardinal.h>
#include <stddef.h>

#define LIST(type, name) \
	size_t name##_count; \
	size_t name##_capacity; \
	size_t name##_max; \
	type *name

#define INIT_LIST(name) (name)=NULL; (name##_max)=0; (name##_count)=0; (name##_capacity)=0;

#include "vm.h"

#define ADD(vm, list, element) ( \
	((list##_capacity) == (list##_count) ? \
		( \
			(list##_capacity) = (list##_capacity) ? (list##_capacity)*2 : 8, \
			(list) = REALLOC(vm, (list), sizeof(*(list))*(list##_capacity)) \
		) \
		: NULL), \
	((list)[(list##_count)++] = (element)), \
	((list##_max) = (list##_max)<(list##_count) ? (list##_count) : (list##_max)), \
	((list##_count) - 1) \
)

#define COUNT(list) (list##_count)
#define TOP(list) ((list)[(list##_count)-1])
#define POP(list) ((list)[--(list##_count)])

#endif
