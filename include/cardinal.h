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

#define CAR_API // @todo replace with the necessary declspecls for DLLs.

typedef struct Car_VM		Car_VM;
typedef struct Car_Config	Car_Config;
typedef struct Car_Version	Car_Version;

typedef struct Car_Traceback_Info Car_Traceback_Info;

typedef void* (Car_Realloc_Fn)(void *ptr, size_t size, void *userdata);
typedef uint64_t (Car_Hash_Fn)(const void *ptr, size_t size);
typedef void (Car_Write_Fn)(Car_VM *vm, const char *message, size_t length);

enum Car_Report_Level {
	CAR_REPORT_LEVEL_DEBUG		= 0,
	CAR_REPORT_LEVEL_NOTE		= 1,
	CAR_REPORT_LEVEL_WARNING	= 2,
	CAR_REPORT_LEVEL_TRACEBACK	= 3,
	CAR_REPORT_LEVEL_ERROR		= 4
};
typedef enum Car_Report_Level Car_Report_Level;

typedef void (Car_Report_Message_Fn)(
		Car_VM *vm,
		int level,
		const char *message
);

typedef void (Car_Report_Source_Fn)(
		Car_VM *vm,
		int level,
		const char *module_name,
		const char *module_path,
		const char *block_name,
		const char *source,
		size_t start,
		size_t count
);

struct Car_Config {
	Car_Hash_Fn *hash_fn;
	Car_Realloc_Fn *realloc_fn;
	Car_Write_Fn *write_fn;
	Car_Report_Message_Fn *report_message_fn;
	Car_Report_Source_Fn *report_source_fn;
	//@todo pad this out for future expansion
	void *userdata;
};

#define CAR_OPT_MODIFY_ALL_CAPS_VAR			0
#define CAR_OPT_CALL_SIGNATURE_WITH_NO_DEFS 1

#define CAR_OPTVAL_IGNORE	0
#define CAR_OPTVAL_WARN		1
#define CAR_OPTVAL_ERROR	2

CAR_API void car_init_bare_config(Car_Config *config);
CAR_API bool car_init_default_config(Car_Config *config, void *userdata);
CAR_API size_t car_get_default_userdata_size();
CAR_API bool car_set_vm_option(Car_VM *vm, int option, int value);
CAR_API int car_get_vm_option(Car_VM *vm, int option);

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

enum Car_Interpret_Result {
	CAR_SUCCESS,
	CAR_COMPILATION_ERROR,
	CAR_RUNTIME_ERROR
};
typedef enum Car_Interpret_Result Car_Interpret_Result;

CAR_API Car_Interpret_Result car_interpret(
		Car_VM *vm,
		const char *module_name,
		const char *module_path,
		const char *source
);

#endif // CARDINAL_H
