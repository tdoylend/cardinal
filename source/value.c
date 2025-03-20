#include <cardinal.h>
#include "vm.h"
#include "value.h"

#include <stdio.h>

Obj *car_new_obj(Car_VM *vm, size_t size, Obj_Type type, Class *class) {
	Obj *obj = ALLOC(vm, size);
	obj->type = type;
	obj->class = class;
	obj->next = vm->objects;
	vm->objects = obj;
	return obj;
}

String *car_new_string_from_bytes(Car_VM *vm, const uint8_t *bytes, size_t count) {
	String *string = (String*)car_new_obj(vm, sizeof(String)+count+1, STRING, vm->string_class);
	memcpy(string->bytes, bytes, count);
	string->bytes[count] = 0;
	string->count = count;
	string->hash = vm->hash_fn(string->bytes, string->count);
	return string;
}

bool car_string_eq_cstring(String *string, const char *cstring) {
	size_t count = strlen(cstring);
	if (string->count != count) return false;
	const uint8_t *cstring_bytes = (const uint8_t*)cstring;
	for (size_t i = 0; i < count; i ++) {
		if (string->bytes[i] != cstring_bytes[i]) return false;
	}
	return true;
}

String *car_new_string_from_cstring(Car_VM *vm, const char *string) {
	return car_new_string_from_bytes(vm, (const uint8_t*)string, strlen(string));
}

Class *car_new_bare_class(Car_VM *vm) {
	Class *class = (Class*)car_new_obj(vm, sizeof(Class), CLASS, vm->class_class);
	return class;
}

Class *car_new_class_and_metaclass(Car_VM *vm, const char *name) {
	const char *suffix = " metaclass";
	char *buffer = ALLOC(vm, strlen(name)+strlen(suffix)+1);
	strcpy(buffer,name);
	strcat(buffer,suffix);

	Class *metaclass = (Class*)car_new_obj(vm, sizeof(Class), CLASS, vm->class_class);
	Class *class = (Class*)car_new_obj(vm, sizeof(Class), CLASS, metaclass);
	class->name = car_new_string_from_cstring(vm,name);
	metaclass->name = car_new_string_from_cstring(vm, buffer);

	FREE(vm,buffer);
	return class;
}

Module *car_get_module(Car_VM *vm, const char *name) {
	for (size_t i = 0; i < COUNT(vm->modules); i ++) {
		if (car_string_eq_cstring(vm->modules[i]->name, name)) {
			return vm->modules[i];
		}
	}
	return NULL;
}

Module *car_new_module(Car_VM *vm, const char *name) {
	Module *module = (Module*)car_new_obj(vm, sizeof(Module), MODULE, NULL);
	module->name = car_new_string_from_cstring(vm, name);
	(void)ADD(vm, vm->modules, module);

	// @todo Create a version of car_declare_global that specifically accepts
	// classes, to reduce duplication of Strings here.
	car_declare_global(vm, module, "System", AS_VALUE(vm->system_class));
	car_declare_global(vm, module, "Class", AS_VALUE(vm->system_class));
	car_declare_global(vm, module, "Object", AS_VALUE(vm->system_class));
	car_declare_global(vm, module, "System", AS_VALUE(vm->system_class));
	return module;
}

bool car_declare_global(Car_VM *vm, Module *module, const char *global, Value value) {
	for (size_t i = 0; i < COUNT(module->global_names); i ++) {
		if (car_string_eq_cstring(module->global_names[i], global)) return false;
	}
	(void)ADD(vm, module->global_names, car_new_string_from_cstring(vm,global));
	(void)ADD(vm, module->globals, value);
	return true;
}

ssize_t car_get_global_index(Car_VM *vm, Module *module, const char *global) {
	for (size_t i = 0; i < COUNT(module->global_names); i ++) {
		if (car_string_eq_cstring(module->global_names[i], global)) return i;
	}
	return -1;
}

Fiber *car_new_fiber(Car_VM *vm) {
	Fiber *fiber = (Fiber*)car_new_obj(vm, sizeof(Fiber), FIBER, NULL);
	fiber->state = SUSPENDED;
	return fiber;
}

