#include "defs.h"
#include <string.h>

CAR_API Car_VM *car_new_vm(Car_Config *config) {
	Car_VM *vm = ALLOC(config, sizeof(Car_VM));	
	memset(vm,0,sizeof(vm));

	vm->hash_fn = config->hash_fn;
	vm->realloc_fn = config->realloc_fn;
	vm->write_fn = config->write_fn;
	vm->report_message_fn = config->report_message_fn;
	vm->report_source_fn = config->report_source_fn;

	return vm;
}

void car_clear_log(Car_VM *vm) {
	vm->log_count = 0;
}

void car_report_log(Car_VM *vm) {
	Log_Entry *log = vm->log;
	for (size_t i = 0; i < vm->log_count; i ++) {
		if (log[i].type == LOG_MESSAGE) {
			Log_Message_Entry entry = log[i].as.message;
			vm->report_message_fn(
				vm,
				entry.level,
				(char*)entry.message->bytes
			);
		} else if (log[i].type == LOG_SOURCE) {
			Log_Source_Entry entry = log[i].as.source;
			vm->report_source_fn(
					vm,
					entry.level,
					(char*)entry.module_name->bytes,
					(char*)entry.module_path->bytes,
					(char*)entry.block_name->bytes,
					(char*)entry.source->bytes,
					entry.start,
					entry.count
			);
		} else {
			UNREACHABLE();
		}
	}
}

CAR_API bool car_compile(Car_VM *vm) {
	return true;
}

CAR_API Car_Interpret_Result car_interpret(
		Car_VM *vm,
		const char *module_name,
		const char *module_path,
		const char *source
) {
	car_clear_log(vm);
	bool success = car_compile(vm);
	car_report_log(vm);
	if (!success) {
		return CAR_COMPILATION_ERROR;
	}

	car_clear_log(vm);

	return CAR_SUCCESS;
}
