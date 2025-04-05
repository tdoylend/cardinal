#include "defs.h"
#include <string.h>

CAR_API Car_VM *car_new_vm(Car_Config *config) {
	Car_VM *vm = ALLOC(config, sizeof(Car_VM));
	memset(vm, 0, sizeof(Car_VM));
	memcpy(vm, config, config->size);

	return vm;
}

static bool compile_single_module(Car_VM *vm, int module_id) {
	return true;
}

static int get_module_id(Car_VM *vm, const char *name) {
	return -1;
}

CAR_API bool car_compile_module(Car_VM *vm, const char *name, const char *source, int *module_id) {
	if (get_module_id(vm, name) != -1) {
		car_
		car_report(vm, NULL, LEVEL_ERROR, "The `$s` module has already been loaded.");
		return false;
	} else {
		if (add_module_source(vm, name, source) < 0) {
			return false;
		}
		for (int i = 0; i < vm->module_count; i ++) {
			if (!compile_single_module(vm, i)) {
				return false;
			}
		}
	}
	return true;
}

CAR_API bool car_run_module(Car_VM *vm, int module_id) {
	return true;
}

CAR_API bool car_compile_and_run(
		Car_VM *vm,
		const char *name,
		const char *source
) {
	int module;
	bool result;

	result = car_compile_module(vm, name, source, &module);

	if (!result) {
		return false;
	}

	result = car_run_module(vm, module);

	return result;
}

