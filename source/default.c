#ifndef CAR_NODEFAULT
#include <cardinal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static CAR_API void *default_realloc_fn(void *ptr, size_t size, void *userdata) {
	if (ptr) {
		if (size) {
			return realloc(ptr,size);
		} else {
			free(ptr);
			return NULL;
		}
	} else {
		if (size) {
			return malloc(size);
		} else {
			return NULL;
		}
	}
}

static CAR_API void default_write_fn(Car_VM *vm, const char *message, size_t length) {
	for (size_t i = 0; i < length; i ++) {
		putc(message[i], stdout);
	}
}

static CAR_API void default_report_fn(Car_VM *vm, const char *message, size_t length) {
	for (size_t i = 0; i < length; i ++) {
		putc(message[i], stderr);
	}
}

CAR_API bool car_init_default_config(Car_Config *config, int size) { 
	car_init_bare_config(config, size);
	config->realloc_fn = default_realloc_fn;
	config->write_fn = default_write_fn;
	config->report_fn = default_report_fn;
	return true;
}
#endif
