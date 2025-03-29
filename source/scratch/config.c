#include <cardinal.h>
#include <string.h>

// The FNV-1a hash is documented here: http://www.isthe.com/chongo/tech/comp/fnv/
// Its creators have dedicated it to the public domain; or, it is licensed via CC-0.
#define FNV_prime 1099511628211ULL
#define FNV_offset_basis 14695981039346656037ULL
static uint64_t hash_fnv_1a(const void *ptr, size_t size) {
	uint64_t hash = FNV_offset_basis;
	for (size_t i = 0; i < size; i ++) {
		hash ^= ((uint8_t*)ptr)[i];
		hash *= FNV_prime;
	}
	return hash;
}

CAR_API void car_init_bare_config(Car_Config *config) {
	memset(config, 0, sizeof(Car_Config));
	config->hash_fn = &hash_fnv_1a;
}

#ifdef CAR_DEFAULT
#include <stdio.h>
#include <stdlib.h>

static void default_write(Car_VM *vm, const char *bytes) {
	printf("%s", bytes);
}

static void *default_realloc(void *ptr, size_t size, void *userdata) {
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
typedef struct {
	size_t unused_;
} Default_Userdata;

CAR_API bool car_init_default_config(Car_Config *config) {
	car_init_bare_config(config);
	config->write_fn = &default_write;
	config->realloc_fn = &default_realloc;
	return true;
}
CAR_API size_t car_get_default_userdata_size() {
	return sizeof(Default_Userdata);
}
#else
CAR_API bool car_init_default_config(Car_Config *config) {
	return false;
}
CAR_API size_t car_get_default_userdata_size() {
	return 0;
}
#endif

