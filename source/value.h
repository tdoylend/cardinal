#ifndef CARDINAL_VALUE_H
#define CARDINAL_VALUE_H

#include <cardinal.h>
#include "list.h"
#include "vm.h"

typedef struct Obj Obj;
typedef struct String String;
typedef struct Class Class;
typedef struct Module Module;
typedef struct Fiber Fiber;
typedef struct Code Code;

typedef union {
	uint64_t bits;
	double value;
} Value;

#define QNAN		0x7FFF000000000000ULL
#define PTR_MASK	0xFFFF000000000000ULL
#define SIGN_BIT	0x8000000000000000ULL

#define NULL_BITS	0x7FFF000000000000ULL

#define VALUE_NULL ((Value){.bits = NULL_BITS})

typedef union {
	uint64_t bits;
	void *ptr;
} ptr_bits_convert;

static inline Value as_value_(Obj *obj) {
	ptr_bits_convert c;
	c.ptr = (void*)obj;
	c.bits |= PTR_MASK;
	return (Value){.bits = c.bits};
}

static inline Obj *as_obj_(Value value) {
	ptr_bits_convert c;
	c.bits = value.bits;
	c.bits &= ~PTR_MASK;
	return (Obj*)(c.ptr);
}

#define AS_VALUE(obj) as_value_((Obj*)(obj))

#define IS_OBJ(value) (((value).bits & PTR_MASK)==PTR_MASK)
#define IS_NUM(value) (((value).bits & QNAN) != QNAN)

#define AS_OBJ(value) (as_obj_((value)))
#define AS_STRING(value) ((String*)(as_obj_((value))))
#define IS_STRING(value) (IS_OBJ(value) && (AS_OBJ(value)->type == STRING))
#define CLASS_OF(vm, value) class_of_((vm),(value))

typedef enum {
	STRING,
	CLASS,
	CODE,
	MODULE,
	FIBER
} Obj_Type;

struct Obj {
	Obj_Type type;
	Class *class;
	Obj *next;
};

static Class *class_of_(Car_VM *vm, Value value) {
	if (IS_OBJ(value)) {
		return AS_OBJ(value)->class;
	} else if (IS_NUM(value)) {
		return vm->num_class; 
	} else {
		switch (value.bits & 0xF) {
			// @todo
		}
		return NULL;
	}
}

Obj *car_new_obj(Car_VM *vm, size_t size, Obj_Type type, Class *class);

struct String {
	Obj obj;
	uint64_t hash;	
	size_t count;
	uint8_t bytes[];
};

String *car_new_string_from_cstring(Car_VM *vm, const char *string);
String *car_new_string_from_bytes(Car_VM *vm, const uint8_t *bytes, size_t count);
bool car_string_eq_cstring(String *string, const char *cstring);

typedef struct Method Method;

typedef enum {
	METHOD_NOT_IMPLEMENTED,
	METHOD_CODE,
	METHOD_BUILTIN
} Method_Type;

typedef void (Builtin_Method)(Car_VM *vm, Fiber *fiber, Value *base);

struct Method {
	Method_Type type;
	union {
		Code *code;
		Builtin_Method *builtin;
	} as;
};

struct Class {
	Obj obj;
	String *name;
	Class *superclass;
	
	LIST(Method, methods);

	int field_count;
};

Class *car_new_class_and_metaclass(Car_VM *vm, const char *name);
Class *car_new_bare_class(Car_VM *vm);

struct Code {
	Obj obj;
	String *source;
	Module *module;
	LIST(uint8_t, bytecode);
	LIST(Value, constants);

	size_t required_stack_space;
};

struct Module {
	Obj obj;
	String *name;
	LIST(Value, globals);
	LIST(String*, global_names);
};

Module *car_get_module(Car_VM *vm, const char *name);
Module *car_new_module(Car_VM *vm, const char *name);
bool car_declare_global(Car_VM *vm, Module *module, const char *global, Value value);
ssize_t car_get_global_index(Car_VM *vm, Module *module, const char *global);

typedef enum {
	SUSPENDED,
	RUNNING,
	ERRORED,
	COMPLETE
} Fiber_State;

typedef struct Frame Frame;
struct Frame {
	Code *code;
	uint8_t *pc;
	Value *base;
	Value *sp;
};

typedef struct Traceback_Line Traceback_Line;
struct Traceback_Line {
	Module *module;
	String *source;
	uint32_t start;
	uint32_t end;
	String *block_name;
};

struct Fiber {
	Obj obj;
	Fiber_State state;
	Fiber *caller;

	LIST(Value, stack);

	LIST(Frame, frames);

	String *error;
	LIST(String*, notes);
	LIST(Traceback_Line, traceback);
	uint32_t error_flags;
};

Fiber *car_new_fiber(Car_VM *vm);

#endif
