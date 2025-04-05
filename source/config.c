#include <cardinal.h>
#include <string.h>

#ifdef CAR_NO_DEFAULT
CAR_API bool car_init_default_config(Car_Config *config, void *userdata) { 
	return false;
}
#endif

#define FNV1A_OFFSET_BASIS (14695981039346656037ULL)
#define FNV1A_PRIME (1099511628211)

static uint64_t builtin_hash_fnv1a(const void *ptr, size_t size) {
	uint64_t hash = FNV1A_OFFSET_BASIS;
	const uint8_t *bytes = ptr;
	for (size_t i = 0; i < size; i++) {
		hash ^= bytes[i];
		hash *= FNV1A_PRIME;
	}
	return hash;
}

CAR_API void car_init_bare_config(Car_Config *config, int size) {
	memset(config, 0, size);
	config->size = size;
	config->hash_fn = &builtin_hash_fnv1a;
}
