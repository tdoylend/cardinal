#include "vm.h"
#include "value.h"

void car_setup_core(Car_VM *vm);

CAR_API Car_VM *car_new_vm(Car_Config *config) {
	Car_VM *vm = ALLOC(config, sizeof(Car_VM));
	vm->hash_fn = config->hash_fn;
	vm->realloc_fn = config->realloc_fn;
	vm->write_fn = config->write_fn;
	vm->error_fn = config->error_fn;
	vm->note_fn = config->note_fn;
	vm->traceback_fn = config->traceback_fn;
	vm->userdata = config->userdata;
	car_setup_core(vm);
	return vm;
}

CAR_API void car_free_vm(Car_VM *vm) {
	FREE(vm, vm);
}

size_t car_ensure_signature(Car_VM *vm, const char *signature) {
	for (size_t i = 0; i < COUNT(vm->signatures); i++) {
		if (car_string_eq_cstring(vm->signatures[i],signature)) {
			return i;
		}
	}
	return ADD(vm, vm->signatures, car_new_string_from_cstring(vm, signature));
}

Code *car_compile(Car_VM *vm, Module *module, Fiber *fiber, String *source);

static bool execute(Car_VM *vm, Fiber *fiber);
#include <stdio.h>
CAR_API Car_Interpret_Result car_interpret(
		Car_VM *vm,
		const char *module_name,
		const char *source_string
) {
	String *source = car_new_string_from_cstring(vm, source_string);
	Module *module = car_get_module(vm, module_name);
	if (!module) module = car_new_module(vm, module_name);
	Fiber *fiber = car_new_fiber(vm);
	Code *code = car_compile(vm, module, fiber, source);
	if (!code) {
		if (fiber->error_flags & CAR_ERROR_DURING_COMPILATION) {
			printf("\x1b[31;1m[Syntax Error]\x1b[m\n\n");
		}
		if (COUNT(fiber->traceback)) {
			Traceback_Line *line = &fiber->traceback[0];
			size_t line_number = 1;
			size_t line_start = line->start;
			char *source = line->source->bytes;

			for (size_t i = 0; i < line->start; i ++) {
				if (source[i] == '\n') {
					line_number ++;
				}
			}
			while (1) {
				if (line_start == 0) break;

				if ((source[line_start] == '\r') || (source[line_start] == '\n')) {
					line_start += 1;
					break;
				}
				line_start --;
			}

			printf("In module `%s`, line %d: \n\n", line->module->name->bytes, line_number);
			printf("    ");
			for (size_t i = line_start; (source[i] != '\n') && (source[i] != '\r') && (source[i] != 0); i++) {
				if ((i >= line->start) && (i < line->end)) printf("\x1b[31;1m");
				else	printf("\x1b[m\x1b[34m");
				putchar(source[i]);
			}
			printf("\x1b[m\n");
			printf("    \x1b[31;1m");
			for (size_t i = line_start; (source[i] != '\n') && (source[i] != '\r') && (source[i] != 0) && (i < line->end); i++) {
				if ((i >= line->start) && (i < line->end)) printf("^");
				else	printf(" ");
			}
			printf("\x1b[m\n\n");
		}
		printf("\x1b[31;1mError:\x1b[m %s\n", fiber->error->bytes);
		for (size_t i = 0; i < COUNT(fiber->notes); i ++) {
			printf("\x1b[36;1mNote:  \x1b[m%s\n", fiber->notes[i]->bytes);
		}
		/*
		for (size_t i = 0; i < COUNT(fiber->error_details); i ++) {
			Error_Line line = fiber->error_details[i];
			if (line.type == TRACEBACK) {
				size_t line_number = 1;
				size_t line_start = line.as.traceback.start - 1;
				char *source = line.as.traceback.source->bytes;

				for (size_t i = 0; i < line.as.traceback.start; i ++) {
					if (source[i] == '\n') {
						line_number ++;
					}
				}
				while (1) {
					if (line_start == 0) break;
					if ((source[line_start] == '\r') || (source[line_start] == '\n')) {
						line_start += 1;
						break;
					}
					line_start --;
				}

				printf("In module `%s`, line %d: \n\n", line.as.traceback.module->name->bytes, line_number);
				printf("    ");
				for (size_t i = line_start; (source[i] != '\n') && (source[i] != '\r') && (source[i] != 0); i++) {
					if ((i >= line.as.traceback.start) && (i < line.as.traceback.end)) printf("\x1b[31;1m");
					else	printf("\x1b[m\x1b[34m");
					putchar(source[i]);
				}
				printf("\x1b[m\n");
				printf("    \x1b[31;1m");
				for (size_t i = line_start; (source[i] != '\n') && (source[i] != '\r') && (source[i] != 0) && (i < line.as.traceback.end); i++) {
					if ((i >= line.as.traceback.start) && (i < line.as.traceback.end)) printf("^");
					else	printf(" ");
				}
				printf("\x1b[m\n\n");

			} else if (line.type == ERROR_MESSAGE) {
				printf("\x1b[31;1mError:\x1b[m ");
				String *string = line.as.message;
				bool inline_code = false;
				for (size_t i = 0; i < string->count; i ++) {
					int c = string->bytes[i];
					if (c == '`') {
						inline_code = !inline_code;
						if (inline_code) {
							printf("\x1b[33m`");
						} else {
							printf("`\x1b[m");
						}
					} else {
						putchar(c);
					}
				}
				printf("\n");
			} else if (line.type == ERROR_NOTE) {
				printf("\x1b[36;1mNote:\x1b[m  ");
				String *string = line.as.note;
				bool inline_code = false;
				for (size_t i = 0; i < string->count; i ++) {
					int c = string->bytes[i];
					if (c == '`') {
						inline_code = !inline_code;
						if (inline_code) {
							printf("\x1b[33m`");
						} else {
							printf("`\x1b[m");
						}
					} else {
						putchar(c);
					}
				}
				printf("\n");
			} else {
				//unreachable
				//@todo
			}
		}*/
	}
	/*while (COUNT(fiber->stack) < code->required_stack_space) {
		(void)ADD(vm, fiber->stack, VALUE_NULL);
	}

	Frame frame;
	frame.code = code;
	frame.pc = code->bytecode;
	frame.base = fiber->stack;
	frame.sp = fiber->stack;
	(void)ADD(vm, fiber->frames, frame);

	execute(vm, fiber);*/

	/*
	if (!code) {
		// compilation error. it is stored in the fiber.
		car_report_error(vm, fiber);
		return CAR_COMPILATION_ERROR;
	}
	vm->current = fiber;
	if (!car_execute(vm, code)) {
		car_report_error(vm, fiber);
		return CAR_RUNTIME_ERROR;
	}*/
	return CAR_SUCCESS;
}

static size_t next_g_(uint8_t **pc) {
	size_t result = 0;
	do {
		uint8_t b = **pc;
		printf("b=%d\n",b);
		(*pc) ++;
		result <<= 7;
		result |= b & 0x7F;
		if (b & 0x80) continue;
	} while(0);
	return result;
}

#include <stdio.h>
#include <inttypes.h>
void debug_stack(Value *bottom, Value *sp) {
	printf("Stack | ");
	for (Value *vptr = bottom; vptr < sp; vptr ++) {
		Value value = *vptr;
		if (IS_OBJ(value)) {
			Obj *obj = AS_OBJ(value);
			switch (obj->type) {
				case STRING: {
					String *string = (String*)obj;
					printf("String<%s> ",string->bytes);
				} break;
				case CLASS: {
					Class *class = (Class*)obj;
					printf("Class<%s> ",class->name->bytes);
				} break;
				case CODE: {
					printf("Code ");
				} break;
				case MODULE: {
					printf("Module ");
				} break;
				case FIBER: {
					printf("Fiber ");
				} break;
			}
		} else {
			printf("%" PRIx64 " ", value.bits);
		}
	}
	printf("\n");
}

#include <stdio.h>
#include "opcodes.h"
static bool execute(Car_VM *vm, Fiber *fiber) {
	Frame *frame = &fiber->frames[COUNT(fiber->frames)-1];

	#define NEXT() (*(frame->pc ++))
	#define PUSH(value) (*(frame->sp ++) = (value))
	#define POP() (*(-- frame->sp))
	#define LOCAL(i) (frame->base[i])
	#define NEXT_g() next_g_(&frame->pc)

	while (true) {
		uint8_t op = NEXT();
		switch (op) {
			case OP_LOAD_GLOBAL_gI: {
				printf("op_load_global_gI\n");
				size_t global = NEXT_g();
				printf("    global = %" PRIx64 "\n", frame->code->module->globals[global].bits);
				PUSH(frame->code->module->globals[global]);
			} break;
			case OP_LOAD_LOCAL_gI: {
				printf("op_load_local_gI\n");
				size_t local = NEXT_g();
				PUSH(LOCAL(local));
			} break;
			case OP_STORE_LOCAL_gI: {
				printf("op_store_local_gI\n");
				size_t local = NEXT_g();
				LOCAL(local) = POP();
			} break;
			case OP_LOAD_CONSTANT_gI: {
				printf("op_load_constant_gI\n");
				size_t constant = NEXT_g();
				printf("    constant = %" PRIx64 "\n", frame->code->constants[constant].bits);
				PUSH(frame->code->constants[constant]);
			} break;
			case OP_CALL_gSignature_gArity: {
				printf("op_call_sig_arity\n");
				size_t signature = NEXT_g();
				size_t arity = NEXT_g();
				Value* stack_frame = &frame->sp[-arity - 1];
				Method method;
				Class *class = CLASS_OF(vm, stack_frame[0]);
				if (COUNT(class->methods) > signature) {
					method = class->methods[signature];
				} else {
					method.type = METHOD_NOT_IMPLEMENTED;
				}
				switch (method.type) {
					case METHOD_NOT_IMPLEMENTED: {
						//@todo do something smart here.
						printf("method not implemented!\n");
					} break;
					case METHOD_CODE: {
						printf("we're not ready yet!\n");
					} break;
					case METHOD_BUILTIN: {
						method.as.builtin(vm, fiber, stack_frame);
					} break;
				}
				frame->sp = &stack_frame[1];
			} break;
			case OP_LOAD_NULL: {
				PUSH(VALUE_NULL);
			} break;
			case OP_RETURN: {
				printf("op_return\n");
				return true;
			} break;
			default: {
				printf("Unknown opcode! %02x\n", op);
			} 
		}
		debug_stack(fiber->stack, frame->sp);
	}
}

