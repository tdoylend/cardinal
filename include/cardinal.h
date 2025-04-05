/*
 * Cardinal -- a simple programming language, inspired by Bob Nystrom's Wren.
 *
 * Compilation options you can define:
 *
 *  - CAR_NO_DEFAULT: Defines the `car_init_default_config(..)` function,
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

#if defined(CAR_DEBUG) && defined(CAR_NO_DEFAULT)
	#error To use CAR_DEBUG, you must also use the default config.
#endif

#define CAR_API __attribute((stdcall)) // @todo replace with the necessary declspecls for DLLs.

typedef struct Car_VM		Car_VM;
typedef struct Car_Config	Car_Config;
typedef struct Car_Version	Car_Version;

typedef struct Car_Traceback_Info Car_Traceback_Info;

typedef void *(Car_Realloc_Fn)(void *ptr, size_t size, void *userdata);
typedef uint64_t (Car_Hash_Fn)(const void *ptr, size_t size);
typedef void (Car_Write_Fn)(Car_VM *vm, const char *message, size_t length);

struct Car_Config {
	Car_Hash_Fn *hash_fn;
	Car_Realloc_Fn *realloc_fn;
	Car_Write_Fn *write_fn;
	Car_Write_Fn *report_fn;

	void *userdata;
	int size;
};

#define CAR_IGNORE							(0)
#define CAR_WARN  							(1)
#define CAR_ERROR 							(2)

#define CAR_DISABLE							(0)
#define CAR_ENABLE							(1)

CAR_API void car_init_bare_config(Car_Config *config, int size);
CAR_API bool car_init_default_config(Car_Config *config, int size);

struct Car_Version {
	int major;
	int minor;
	int patch;
};

CAR_API int car_compare_versions(Car_Version *a, Car_Version *b);

#define CAR_COMPILED_VERSION ((Car_Version){ 0, 0, 1 })
CAR_API Car_Version car_get_linked_version(void);

CAR_API Car_VM *car_new_vm(Car_Config *config);

CAR_API bool car_set_option(Car_VM *vm, int option, int value);
CAR_API int  car_get_option(Car_VM *vm, int option);

CAR_API void car_free_vm(Car_VM *vm);

CAR_API bool car_compile_and_run(Car_VM *vm, const char *name, const char *source);
CAR_API bool car_compile(Car_VM *vm, const char *name, const char *source, int *module_id);
CAR_API bool car_run_module(Car_VM *vm, int module_id);

CAR_API int  car_get_module_count(Car_VM *vm);
CAR_API const char *car_get_module_name(Car_VM *vm);

#endif // CARDINAL_H
