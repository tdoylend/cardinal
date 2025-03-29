#ifndef DEFS_H
#define DEFS_H

#include <cardinal.h>

typedef struct String String;

struct String {
	uint64_t hash;
	uint64_t length;
	uint8_t bytes[];
};

typedef struct Log_Entry Log_Entry;

struct Car_VM {
	Car_Hash_Fn *hash_fn;
	Car_Realloc_Fn *realloc_fn;
	Car_Write_Fn *write_fn;
	Car_Report_Message_Fn *report_message_fn;
	Car_Report_Source_Fn *report_source_fn;

	void *userdata;

	Log_Entry *log;
	int log_count;
	int log_capacity;
};

typedef struct Log_Message_Entry Log_Message_Entry;
typedef struct Log_Source_Entry Log_Source_Entry;

enum Log_Entry_Type {
	LOG_MESSAGE,
	LOG_SOURCE
};
typedef enum Log_Entry_Type Log_Entry_Type;

struct Log_Message_Entry {
	int level;
	String *message;
};

struct Log_Source_Entry {
	int level;
	String *module_name;
	String *module_path;
	String *block_name;
	String *source;
	size_t start;
	size_t count;
};

struct Log_Entry {
	Log_Entry_Type type;
	union {
		Log_Message_Entry message;
		Log_Source_Entry source;
	} as;
};

#define ALLOC(vm, size) (vm->realloc_fn(NULL,(size), vm->userdata))

void car_report_unreachable(const char *file, int line, const char *function);
#define UNREACHABLE() car_report_unreachable(__FILE__,__LINE__,__func__);

#endif
