#ifndef CARDINAL_VM_H
#define CARDINAL_VM_H

#include <cardinal.h>
#include <string.h>
#include "list.h"

typedef struct Obj Obj;
typedef struct String String;
typedef struct Class Class;
typedef struct Module Module;
typedef struct Fiber Fiber;
typedef struct Code Code;

struct Car_VM {
	Car_Hash_Fn *hash_fn;
	Car_Realloc_Fn *realloc_fn;
	Car_Message_Fn *write_fn;
	Car_Error_Fn *error_fn;
	Car_Message_Fn *note_fn;
	Car_Traceback_Fn *traceback_fn;

	void *userdata;

	Obj *objects;

	LIST(String*, signatures);

	LIST(Module*, modules);

	Class *string_class;
	Class *class_class;
	Class *object_class;
	Class *system_class;
	Class *num_class;
};

size_t car_ensure_signature(Car_VM *vm, const char *signature);

#define ALLOC_(vm, size) (memset((vm)->realloc_fn( \
		NULL, \
		(size), \
		(vm)->userdata \
	),0,(size)))
#define REALLOC_(vm, ptr, size) ((vm)->realloc_fn((ptr),(size),(vm)->userdata))
#define FREE_(vm, ptr) ((vm)->realloc_fn((ptr),0,(vm)->userdata))

#ifdef CAR_DEBUG
#define ALLOC(vm, size) (PUSH_LINE_INFO, ALLOC_(vm, size))
#define REALLOC(vm, ptr, size) (PUSH_LINE_INFO, REALLOC_(vm, ptr, size))
#define FREE(vm, size) (PUSH_LINE_INFO, ALLOC_(vm, size))
#else
#define ALLOC ALLOC_
#define REALLOC REALLOC_
#define FREE FREE_
#endif

#endif // CARDINAL_VM_H
