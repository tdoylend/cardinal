#include <cardinal.h>
#include "vm.h"
#include "value.h"

static void static_System__write__1(Car_VM *vm, Fiber *fiber, Value *args) {
	vm->write_fn(vm, AS_STRING(args[1])->bytes);
	args[0] = VALUE_NULL;
	return;
}

static void Num__op_range_excl(Car_VM *vm, Fiber *fiber, Value *args) {
	if (IS_NUM(args[1])) {
		//Range *range
		//args[0] = AS_VALUE(range);
	} else {
		//@todo error or sumthin
	}
}

void bind_builtin(Car_VM *vm, Class *class, const char *signature, Builtin_Method method) {
	size_t id = car_ensure_signature(vm, signature);
	while (COUNT(class->methods) <= id) {
		(void)ADD(vm, class->methods, (Method){.type=METHOD_NOT_IMPLEMENTED});
	}
	class->methods[id].type = METHOD_BUILTIN;
	class->methods[id].as.builtin = method;
}

void car_setup_core(Car_VM *vm) {
	Class *string_metaclass = car_new_bare_class(vm); 
	vm->string_class = car_new_bare_class(vm);
	vm->class_class = car_new_bare_class(vm);

	vm->string_class->obj.class = string_metaclass;
	vm->class_class->obj.class = vm->class_class;
	string_metaclass->obj.class = vm->class_class;

	vm->string_class->name = car_new_string_from_cstring(vm, "String");
	string_metaclass->name = car_new_string_from_cstring(vm, "String metaclass");
	vm->class_class->name = car_new_string_from_cstring(vm, "Class");

	vm->system_class = car_new_class_and_metaclass(vm, "System");
	vm->num_class = car_new_class_and_metaclass(vm, "Num");

	bind_builtin(vm, vm->system_class->obj.class, "write(_)", static_System__write__1);
}
