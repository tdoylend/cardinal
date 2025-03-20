/*
 * Cardinal -- a simple programming language, inspired by Bob Nystrom's Wren.
 *
 * Compilation options you can define:
 *
 *  - CAR_DEFAULT: Defines the `car_init_default_config(..)` function,
 *    which provides useful defaults for the config functions.
 *  - CAR_DEBUG: Enables debugging for the VM itself. This only works correctly
 *    when the default configuration is enabled.
 *  - CAR_DLL: Enables DLL export.
*/

#ifndef CARDINAL_H
#define CARDINAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(CAR_DEBUG) && !defined(CAR_DEFAULT)
	#error To use CAR_DEBUG, you must also use the default config with `CAR_DEFAULT`.
#endif

#define CAR_API // @todo replace with the necessary declspecls for DLLs.

typedef struct Car_VM		Car_VM;
typedef struct Car_Config	Car_Config;
typedef struct Car_Version	Car_Version;

typedef struct Car_Traceback_Info Car_Traceback_Info;

typedef void* (Car_Realloc_Fn)(void *ptr, size_t size, void *userdata);
typedef uint64_t (Car_Hash_Fn)(const void *ptr, size_t size);
typedef void (Car_Message_Fn)(Car_VM *vm, const char *message);
typedef void (Car_Traceback_Fn)(Car_VM *vm, Car_Traceback_Info *info);
typedef void (Car_Error_Fn)(Car_VM *vm, const char *error, uint32_t flags);

#define CAR_ERROR_RAISED_EXPLICITLY		(0x0001)
#define CAR_ERROR_DURING_COMPILATION    (0x0002)

struct Car_Traceback_Info {
	const char *module;
	const char *source;
	size_t start;
	size_t end;
	const char *function;
};

struct Car_Config {
	Car_Hash_Fn *hash_fn;
	Car_Realloc_Fn *realloc_fn;
	Car_Message_Fn *write_fn;
	Car_Error_Fn *error_fn;
	Car_Message_Fn *note_fn;
	Car_Traceback_Fn *traceback_fn;

	void *userdata;
};

CAR_API void car_init_bare_config(Car_Config *config);
CAR_API bool car_init_default_config(Car_Config *config);
CAR_API size_t car_get_default_userdata_size();

struct Car_Version {
	int major;
	int minor;
	int patch;
};

CAR_API int car_compare_versions(Car_Version *a, Car_Version *b);

#define CAR_COMPILED_VERSION ((Car_Version){ 0, 0, 1 })
CAR_API Car_Version car_get_linked_version(void);

CAR_API Car_VM *car_new_vm(Car_Config *config);
CAR_API void car_free_vm(Car_VM *vm);

typedef enum {
	CAR_SUCCESS,
	CAR_COMPILATION_ERROR,
	CAR_RUNTIME_ERROR
} Car_Interpret_Result;

CAR_API Car_Interpret_Result car_interpret(Car_VM *vm, const char *module, const char *source);

#endif // CARDINAL_H
