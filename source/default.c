#ifndef CAR_NODEFAULT
#include <cardinal.h>
#include <stdlib.h>
#include <string.h>

CAR_API void *realloc_fn(void *ptr, size_t size, void *userdata) {
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

CAR_API bool car_init_default_config(Car_Config *config, void *userdata) { 
	memset(config, 0, sizeof(Car_Config));	
	config->realloc_fn = realloc_fn;
	config->userdata = userdata;
	return true;
}
#endif
