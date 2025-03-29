#include <cardinal.h>
#include "vm.h"
#include "value.h"
#include <stdio.h>
#include "list.h"
#include <stdarg.h>
#include "opcodes.h"

static void emit(Car_VM *vm, Code *code, uint8_t value) {
	(void)ADD(vm, code->bytecode, value);
}

static void emit_g(Car_VM *vm, Code *code, size_t value) {
	uint8_t words[(sizeof(size_t)*8) / 7 + 1];
	size_t count = 0;
	while (value) {
		words[count++] = value & 0x7F;
		value >>= 7;
	}
	if (count) {
		while (count) {
			--count;
			emit(vm, code, words[count] | (count ? 0x80 : 0x00));
		}
	} else {
		emit(vm, code, 0);
	}
}

static void emit_16(Car_VM *vm, Code *code, uint16_t value) {
	emit(vm, code, value & 0xFF);
	emit(vm, code, value >> 8);
}

static size_t ensure_constant(Car_VM *vm, Code *code, Value value) {
	// @todo intern strings rather than using bitwise compare
	for (size_t i = 0; i < COUNT(code->constants); i++) {
		if (code->constants[i].bits == value.bits) {
			return i;
		}
	}
	return ADD(vm, code->constants, value);
}

typedef struct Stream Stream;
struct Stream {
	String *source;
	size_t index;
};

static char next_char(Stream *stream) {
	if (stream->index >= stream->source->count) {
		return 0;
	} else {
		return stream->source->bytes[stream->index++];
	}
}

static char peek_char(Stream *stream) {
	Stream temp = *stream;
	return next_char(&temp);
}

static bool match(Stream *stream, const char *string) {
	Stream temp = *stream;
	for (const char *s = string; *s; s ++) {
		char c = *s;
		if (peek_char(&temp) == c) {
			(void)next_char(&temp);
		} else {
			return false;
		}
	}
	*stream = temp;
	return true;
}

static bool is_ident_start(int c) {
	return ((c >= 'A') && (c <= 'Z'))
		|| ((c >= 'a') && (c <= 'z'))
		|| (c == '_');
}

static bool is_ident_any(int c) {
	return is_ident_start(c)
		|| ((c >= '0') && (c <= '9'));
}

static bool is_hex_digit(int c) {
	return ((c >= '0') && (c <= '9'))
		|| ((c >= 'a') && (c <= 'f'))
		|| ((c >= 'A') && (c <= 'F'));
}

typedef enum {
	#define TOKEN(x,...) TOKEN_##x,
	#include "tokens.h"
	#undef TOKEN
	TOKEN_MAX
} Token_Type;

typedef struct Token Token;
struct Token {
	Token_Type type;
	uint32_t start;
	uint32_t end;
};

typedef struct Compile_Context Compile_Context;
struct Compile_Context {
	Car_VM *vm;
	Module *module;
	Fiber *fiber;
	String *source;
	Stream *stream;
};


static bool token_eq_kw(String *source, Token token, const char *keyword) {
	size_t length = token.end - token.start;
	if (length != strlen(keyword)) return false;
	return strncmp(&source->bytes[token.start], keyword, length) == 0;
}

static bool token_eq(String *source, Token a, Token b) {
	size_t length = a.end - a.start;
	if ((b.end-b.start) != length) return false;
	return memcpy(&source->bytes[a.start], &source->bytes[b.start], length);
}

static void eat_whitespace(Stream *stream) {
	int comment_level = 0;
	while (1) {
		if (comment_level < 0) {
			// single line comment
			char next = peek_char(stream);
			if ((next == '\r') || (next == '\n') || (next == 0)) {
				break;
			} else {
				(void)next_char(stream);
			}
		} else if (comment_level > 0) {
			// multi line comment
			if (match(stream, "*/")) {
				comment_level --;
			} else if (match(stream, "/*")) {
				comment_level ++;
			} else {
				(void)next_char(stream);
			}
		} else {
			// not in comment
			if (match(stream, "/*")) {
				comment_level = 1;
			} else if (match(stream, "//")) {
				comment_level = -1;
			} else {
				char next = peek_char(stream);
				if ((next == ' ') || (next == '\t')) {
					(void)next_char(stream);
				} else {
					break;
				}
			}
		}
	}
}

static Token next_token(Stream *stream) {
	eat_whitespace(stream);

	Token result = {
		.start = stream->index
	};

	if      (peek_char(stream) == 0)	result.type = TOKEN_EOF;
	else if (match(stream, "\"\"\""))	result.type = TOKEN_TRIPLE_QUOTE;
	else if (match(stream, "\""))		result.type = TOKEN_QUOTE;
	else if (match(stream, "("))		result.type = TOKEN_OPEN_PAREN;
	else if (match(stream, ")"))		result.type = TOKEN_CLOSE_PAREN;
	else if (match(stream, "["))		result.type = TOKEN_OPEN_SQUARE;
	else if (match(stream, "]"))		result.type = TOKEN_CLOSE_SQUARE;
	else if (match(stream, ".")) {
		if (match(stream,".")) {
			if (match(stream, ".<")) {
				result.type = TOKEN_DOT_DOT_LESS;
			} else {
				result.type = TOKEN_DOT_DOT;
			}
		} else {
			result.type = TOKEN_DOT;
		}
	}
	else if (match(stream, "{"))		result.type = TOKEN_OPEN_CURLY;
	else if (match(stream, "}"))		result.type = TOKEN_CLOSE_CURLY;
	else if (match(stream, "-")) {
		if (match(stream, "=")) {
			result.type = TOKEN_MINUS_EQUALS;
		} else {
			result.type = TOKEN_MINUS;
		}
	}
	else if (match(stream, "!")) {
		if (match(stream, "=")) {
			result.type = TOKEN_BANG_EQUALS;
		} else {
			result.type = TOKEN_BANG;
		}
	}
	else if (match(stream, "~"))		result.type = TOKEN_TILDE;
	else if (match(stream, "*")) {
		if (match(stream, "=")) {
			result.type = TOKEN_ASTERISK_EQUALS;
		} else {
			result.type = TOKEN_ASTERISK;
		}
	}
	else if (match(stream, "/")) {
		if (match(stream, "=")) {
			result.type = TOKEN_SLASH_EQUALS;
		} else {
			result.type = TOKEN_SLASH;
		}
	}
	else if (match(stream, "%")) {
		if (match(stream, "=")) {
			result.type = TOKEN_PERCENT_EQUALS;
		} else {
			result.type = TOKEN_PERCENT;
		}
	} 
	else if (match(stream, "+")) {
		if (match(stream, "=")) {
			result.type = TOKEN_PLUS_EQUALS;
		} else {
			result.type = TOKEN_PLUS;
		}
	}
	else if (match(stream, "<")) {
		if (match(stream, "<")) {
			if (match(stream, "=")) {
				result.type = TOKEN_LESS_LESS_EQUALS;
			} else {
				result.type = TOKEN_LESS_LESS;
			}
		} else if (match(stream, "=")) {
			result.type = TOKEN_LESS_EQUALS;
		} else {
			result.type = TOKEN_LESS;
		}
	}
	else if (match(stream, ">")) {
		if (match(stream, ">")) {
			if (match(stream, "=")) {
				result.type = TOKEN_GREATER_GREATER_EQUALS;
			} else {
				result.type = TOKEN_GREATER_GREATER;
			}
		} else if (match(stream, "=")) {
			result.type = TOKEN_GREATER_EQUALS;
		} else {
			result.type = TOKEN_GREATER;
		}
	}
	else if (match(stream, "&")) {
		if (match(stream, "&")) {
			if (match(stream, "=")) {
				result.type = TOKEN_AMPERSAND_AMPERSAND_EQUALS;
			} else {
				result.type = TOKEN_AMPERSAND_AMPERSAND;
			}
		} else if (match(stream, "=")) {
			result.type = TOKEN_AMPERSAND_EQUALS;
		} else {
			result.type = TOKEN_AMPERSAND;
		}
	}
	else if (match(stream, "|")) {
		if (match(stream, "|")) {
			if (match(stream, "=")) {
				result.type = TOKEN_PIPE_PIPE_EQUALS;
			} else {
				result.type = TOKEN_PIPE_PIPE;
			}
		} else if (match(stream, "=")) {
			result.type = TOKEN_PIPE_EQUALS;
		} else {
			result.type = TOKEN_PIPE;
		}
	}
	else if (match(stream, "0x")) {
		while (is_hex_digit(peek_char(stream))) {
			(void)next_char(stream);
		}
		result.type = TOKEN_HEX_NUMBER;
	}
	else if ((peek_char(stream) >= '0') && (peek_char(stream) <= '9')) {
		bool found_exponent = false;
		bool found_dot = false;
		//@todo don't eat the dot if it is trailing -- e.g., 
		// "0." should only parse as `0` not `0.`.

		do {
			if ((peek_char(stream) >= '0') && (peek_char(stream) <= '9')) {
				(void)next_char(stream);
				continue;
			} else if (!found_exponent && !found_dot && (peek_char(stream) == '.')) {
				Stream temp = *stream;
				(void)next_char(&temp);
				if ((peek_char(&temp) >= '0') && (peek_char(&temp) <= '9')) {
					found_dot = true;
					(void)next_char(stream);
					continue;
				}
			} else if (!found_exponent && ((peek_char(stream) == 'e') || (peek_char(stream) == 'E'))) {
				Stream temp = *stream;
				(void)next_char(&temp);
				if ((peek_char(&temp) >= '0') && (peek_char(&temp) <= '9')) {
					found_exponent = true;
					(void)next_char(stream);
					continue;
				}
			}
		} while (0);

		result.type = TOKEN_DEC_NUMBER;
	}
	else if (match(stream, "^"))		result.type = TOKEN_CARET;
	else if (match(stream, "=")) {
		if (match(stream, "=")) {
			result.type = TOKEN_EQUALS_EQUALS;
		} else {
			result.type = TOKEN_EQUALS;
		}
	}
	else if (match(stream, "?"))		result.type = TOKEN_QUESTION;
	else if (match(stream, ":"))		result.type = TOKEN_COLON;
	else if (is_ident_start(peek_char(stream))) {
		result.end = result.start;
		while (is_ident_any(peek_char(stream))) {
			result.end ++;
			(void)next_char(stream);
		}

		if      (token_eq_kw(stream->source,result,"as"))			result.type = TOKEN_AS;
		else if (token_eq_kw(stream->source,result,"break"))		result.type = TOKEN_BREAK;
		else if (token_eq_kw(stream->source,result,"class"))		result.type = TOKEN_CLASS;
		else if (token_eq_kw(stream->source,result,"construct"))	result.type = TOKEN_CONSTRUCT;
		else if (token_eq_kw(stream->source,result,"continue"))	result.type = TOKEN_CONTINUE;
		else if (token_eq_kw(stream->source,result,"else"))		result.type = TOKEN_ELSE;
		else if (token_eq_kw(stream->source,result,"false"))		result.type = TOKEN_FALSE;
		else if (token_eq_kw(stream->source,result,"field"))		result.type = TOKEN_FIELD;
		else if (token_eq_kw(stream->source,result,"for"))			result.type = TOKEN_FOR;
		else if (token_eq_kw(stream->source,result,"foreign"))		result.type = TOKEN_FOREIGN;
		else if (token_eq_kw(stream->source,result,"if"))			result.type = TOKEN_IF;
		else if (token_eq_kw(stream->source,result,"import"))		result.type = TOKEN_IMPORT;
		else if (token_eq_kw(stream->source,result,"in"))			result.type = TOKEN_IN;
		else if (token_eq_kw(stream->source,result,"is"))			result.type = TOKEN_IS;
		else if (token_eq_kw(stream->source,result,"null"))		result.type = TOKEN_NULL;
		else if (token_eq_kw(stream->source,result,"return"))		result.type = TOKEN_RETURN;
		else if (token_eq_kw(stream->source,result,"static"))		result.type = TOKEN_STATIC;
		else if (token_eq_kw(stream->source,result,"super"))		result.type = TOKEN_SUPER;
		else if (token_eq_kw(stream->source,result,"this"))		result.type = TOKEN_THIS;
		else if (token_eq_kw(stream->source,result,"true"))		result.type = TOKEN_TRUE;
		else if (token_eq_kw(stream->source,result,"var"))			result.type = TOKEN_VAR;
		else if (token_eq_kw(stream->source,result,"while"))		result.type = TOKEN_WHILE;
		else {
			result.type = TOKEN_IDENTIFIER;
		}

	}
	else if (match(stream, "\n")) {
		(void)match(stream,"\r");
		Stream temp = *stream;
		eat_whitespace(&temp);
		if (match(&temp, ".") && !match(&temp,".")) {
			result.type = TOKEN_DOT;
			*stream = temp;
		} else {
			result.type = TOKEN_NEWLINE;
		}
	}
	else if (match(stream, "\r")) {
		(void)match(stream,"\n");
		Stream temp = *stream;
		eat_whitespace(&temp);
		if (match(&temp, ".") && !match(&temp,".")) {
			result.type = TOKEN_DOT;
			*stream = temp;
		} else {
			result.type = TOKEN_NEWLINE;
		}
	}
	else {
		(void)next_char(stream);
		result.type = TOKEN_UNKNOWN;
	}

	result.end = stream->index;
	return result;
}

static Token peek_token(Stream *stream) {
	Stream temp = *stream;
	return next_token(&temp);
}

static bool match_token_type(Stream *stream, Token_Type type) {
	if (peek_token(stream).type == type) {
		(void)next_token(stream);
		return true;
	}
	return false;
}

static const char *token_name(Token token) {
	switch (token.type) {
		#define TOKEN(x) case TOKEN_##x: return "TOKEN_" #x;
		#include "tokens.h"
		#undef TOKEN
	}
	return NULL;
}

Value token_as_value(Compile_Context *ctx, Token token) {
	return AS_VALUE(car_new_string_from_bytes(ctx->vm, &ctx->source->bytes[token.start], token.end - token.start));
}

String *vformat(Compile_Context *ctx, const char *format, va_list args) {
	va_list args2;
	va_copy(args2, args);
	size_t count = 0;
	for (const char *s = format; *s; s ++) {
		char c = *s;
		if (c == '@') {
			String *substring = AS_STRING(va_arg(args2, Value));
			count += substring->count;
		} else if (c == '$') {
			count += strlen(va_arg(args2, const char*));
		} else if (c == '^') {
			Token token = va_arg(args2, Token);
			count += (token.end - token.start);
		} else {
			count ++;
		}
	}
	va_end(args2);
	String *string = (String*)car_new_obj(
			ctx->vm,
			sizeof(String)+count+1,
			STRING,
			ctx->vm->string_class
	);
	string->count = count;
	char *out = string->bytes;
	for (const char *s = format; *s; s ++) {
		char c = *s;
		if (c == '@') {
			String *substring = AS_STRING(va_arg(args, Value));
			memcpy(out, substring->bytes, substring->count);
			out += substring->count;
		} else if (c == '$') {
			const char *substring = va_arg(args, const char *);
			size_t count = strlen(substring);
			memcpy(out, substring, count);
			out += count;
		} else if (c == '^') {
			Token token = va_arg(args, Token);
			size_t count = token.end - token.start;
			memcpy(out, &ctx->source->bytes[token.start], count);
			out += count;
		} else {
			*(out++) = c;
		}
	}
	string->hash = ctx->vm->hash_fn(string->bytes, string->count);
	return string;
}

static void set_error(Compile_Context *ctx, Token *token, const char *format, ...) {
	va_list args;
	va_start(args, format);
	String *string = vformat(ctx, format, args);
	va_end(args);

	ctx->fiber->state = ERRORED;
	ctx->fiber->error = string;
	ctx->fiber->error_flags = CAR_ERROR_DURING_COMPILATION;
	if (token) {	
		Traceback_Line traceback = {ctx->module,ctx->source,token->start,token->end,NULL};
		ADD(ctx->vm, ctx->fiber->traceback, traceback);
	} else {
		// Maybe we should set a flag here?
	}
}

static void add_note(Compile_Context *ctx, const char *format, ...) {
	va_list args;
	va_start(args, format);
	String *string = vformat(ctx, format, args);
	va_end(args);
	ADD(ctx->vm, ctx->fiber->notes, string);
}

Code *display_unknown_token_error(Compile_Context *ctx) {
	Token token = peek_token(ctx->stream);
	String *source = ctx->stream->source;
	set_error(ctx, &token, "This symbol is not defined in Cardinal.");
	if (token_eq_kw(source,token,"`") || token_eq_kw(source,token,"'")) {
		add_note(ctx, "Only the double quote (\") is used for strings -- not the backtick"
					  "(`) or the single quote (').");
	}
	return NULL;
}

typedef enum {
	OPER_ADD
} Operator;

const Token_Type group_binary[] = {
	TOKEN_PLUS, TOKEN_MINUS, TOKEN_ASTERISK, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_LESS_LESS,
	TOKEN_LESS_EQUALS, TOKEN_LESS, TOKEN_GREATER, TOKEN_GREATER_GREATER, TOKEN_GREATER_EQUALS,
	TOKEN_AMPERSAND, TOKEN_AMPERSAND_AMPERSAND, TOKEN_PIPE_PIPE, TOKEN_PIPE, TOKEN_CARET,
	TOKEN_EQUALS_EQUALS, TOKEN_BANG_EQUALS, TOKEN_DOT_DOT, TOKEN_DOT_DOT_LESS,
	TOKEN_IS,

	TOKEN_MAX
};

const Token_Type group_reserved[] = {
	//@todo
};

const Token_Type group_assignment[] = {
	TOKEN_EQUALS, TOKEN_PLUS_EQUALS, TOKEN_MINUS_EQUALS, TOKEN_ASTERISK_EQUALS, TOKEN_SLASH_EQUALS,
	TOKEN_PERCENT_EQUALS, TOKEN_AMPERSAND_EQUALS, TOKEN_PIPE_EQUALS, TOKEN_CARET_EQUALS,
	TOKEN_LESS_LESS_EQUALS, TOKEN_GREATER_GREATER_EQUALS, TOKEN_AMPERSAND_AMPERSAND_EQUALS,
	TOKEN_PIPE_PIPE_EQUALS,

	TOKEN_MAX
};

const Token_Type expression_start[] = {
	TOKEN_QUOTE, TOKEN_TRIPLE_QUOTE, TOKEN_OPEN_PAREN, TOKEN_OPEN_SQUARE, TOKEN_OPEN_CURLY,
	TOKEN_DEC_NUMBER, TOKEN_HEX_NUMBER, TOKEN_MINUS, TOKEN_TILDE, TOKEN_BANG,
	TOKEN_IDENTIFIER, 

	TOKEN_MAX
};

const Token_Type group_statement_start[] = {
	TOKEN_MAX
};

const Token_Type group_prefix_unary[] = {
	TOKEN_MINUS, TOKEN_TILDE, TOKEN_BANG,

	TOKEN_MAX
};

static bool in_group(Token token, const Token_Type *group) {
	for (size_t i = 0; group[i] != TOKEN_MAX; i ++) {
		if (token.type == group[i]) return true;
	}
	return false;
}

static Token token_union(Token a, Token b) {
	Token result = {
		TOKEN_MERGED,
		a.start < b.start ? a.start : b.start,
		a.end > b.end ? a.end : b.end
	};
	return result;
}

#define ST_START				0x0100
#define ST_MODULE				0x0200
#define ST_STATEMENT			0x0300
#define ST_IF					0x0400
#define ST_WHILE				0x0500
#define ST_EXPRESSION			0x0600
#define ST_BLOCK_OR_STATEMENT	0x0700
#define ST_CONDITION			0x0800
#define ST_FOR					0x0900
#define ST_VAR					0x0A00

#define SU_SETUP				0x00
#define SU_FINISH				0x01
#define SU_MONO					0x02
#define SU_INTER				0x03

/*
typedef struct Stack_Value Stack_Value;
struct Stack_Value {
	Token token;
	bool is_local;
};
*/

#include <stdlib.h>
Code *car_compile(Car_VM *vm, Module *module, Fiber *fiber, String *source) {
	Code *code = (Code*)car_new_obj(vm, sizeof(Code), CODE, NULL);
	Stream stream = {source};
	Compile_Context ctx = {vm, module, fiber, source, &stream};

	if (source->count > 0xFFFFFFFEUL) {
		set_error(&ctx, NULL, "The source code of the `@` module is too long.", AS_VALUE(module->name));
		add_note(&ctx, "The maximum allowable length is 2^32-2 bytes (~4 GB).");
		return NULL;
	}

	LIST(uint32_t, states);
	INIT_LIST(states);

	LIST(Token, stack_values);
	INIT_LIST(stack_values);

	ADD(vm, states, ST_START);

	while (COUNT(states)) {
		uint32_t state = POP(states);
		Token token = peek_token(&stream);
		switch (state) {
			case ST_START: {
				ADD(vm, states, ST_MODULE);
			} break;
			case ST_MODULE: {
				if (token.type != TOKEN_EOF) {
					ADD(vm, states, ST_MODULE);
					ADD(vm, states, ST_STATEMENT);
				}
			} break;
			case ST_STATEMENT: {
				if (in_group(token, expression_start)) {
					ADD(vm, states, ST_STATEMENT | SU_FINISH);
					ADD(vm, states, ST_EXPRESSION);
				} else if (token.type == TOKEN_NEWLINE) {
					(void)next_token(&stream);
				} else if (match_token_type(&stream, TOKEN_IF)) {
					ADD(vm, states, ST_IF);
				} else if (match_token_type(&stream, TOKEN_WHILE)) {
					ADD(vm, states, ST_WHILE);
				} else if (match_token_type(&stream, TOKEN_FOR)) {
					ADD(vm, states, ST_FOR);
				} else if (match_token_type(&stream, TOKEN_VAR)) {
					ADD(vm, states, ST_VAR);
				} else {
					set_error(&ctx, &token, "A statement is expected here.");
					return NULL;
				}
			} break;
			case ST_STATEMENT | SU_FINISH: {
				// discard here
				if ((token.type == TOKEN_NEWLINE) || (token.type == TOKEN_EOF)) {
					(void)next_token(&stream);
				} else {
					set_error(&ctx, &token, "A newline or EOF is expected here.");
					return NULL;
				}
			} break;
			case ST_FOR: {
				if (!match_token_type(&stream, TOKEN_OPEN_PAREN)) {
					set_error(&ctx, &token, "The range clause on a `for` loop must be enclosed "
							"in parentheses.");
					return NULL;
				}
				if (peek_token(&stream).type != TOKEN_IDENTIFIER) {
					set_error(&ctx, &token, "You must provide a name for the loop variable.");
					return NULL;
				}
				(void)next_token(&stream);
				if (!match_token_type(&stream, TOKEN_IN)) {
					set_error(&ctx, &token, "Separate the variable and range with the `in` "
							"keyword.");
					return NULL;
				}
				ADD(vm, states, ST_FOR | SU_INTER);
				ADD(vm, states, ST_EXPRESSION);
			} break;
			case ST_FOR | SU_INTER: {
				if (!match_token_type(&stream, TOKEN_CLOSE_PAREN)) {
					set_error(&ctx, &token, "A close-paren is missing here to enclose the "
							"range clause of the `for` statement.");
					return NULL;
				}
				ADD(vm, states, ST_FOR | SU_FINISH);
				ADD(vm, states, ST_BLOCK_OR_STATEMENT);
			} break;
			case ST_FOR | SU_FINISH: {
			} break;
			case ST_VAR: {
				if (token.type != TOKEN_IDENTIFIER) {
					set_error(&ctx, &token, "A variable name is required here.");
					return NULL;
				}
				(void)next_token(&stream);
				ADD(vm, states, ST_VAR | SU_FINISH);
				if (match_token_type(&stream, TOKEN_EQUALS)) {
					ADD(vm, states, ST_EXPRESSION);
				} else {
					// nul
				}
			} break;
			case ST_VAR | SU_FINISH: {
				if ((token.type == TOKEN_NEWLINE) || (token.type == TOKEN_EOF)) {
					(void)next_token(&stream);
				} else {
					set_error(&ctx, &token, "End-of-line is expected here.");
					return NULL;
				}
			} break;
			case ST_IF: {
				ADD(vm, states, ST_IF | SU_INTER);
				ADD(vm, states, ST_CONDITION);
			} break;
			case ST_IF | SU_INTER: {
				ADD(vm, states, ST_IF | SU_FINISH);
				ADD(vm, states, ST_BLOCK_OR_STATEMENT);
			} break;
			case ST_IF | SU_FINISH: {
			} break;
			case ST_WHILE: {
				ADD(vm, states, ST_WHILE | SU_INTER);
				ADD(vm, states, ST_CONDITION);
			} break;
			case ST_WHILE | SU_INTER: {
				ADD(vm, states, ST_WHILE | SU_FINISH);
				ADD(vm, states, ST_BLOCK_OR_STATEMENT);
			} break;
			case ST_WHILE | SU_FINISH: {
			} break;
			case ST_BLOCK_OR_STATEMENT: {
				if (match_token_type(&stream, TOKEN_OPEN_CURLY)) {
					set_error(&ctx, &token, "We don't know how to handle blocks yet!");
					return NULL;
				} else {
					ADD(vm, states, ST_STATEMENT);
				}
			} break;
			case ST_CONDITION: {
				if (!match_token_type(&stream, TOKEN_OPEN_PAREN)) {
					Token try_merge = token;
					while (1) {
						if (match_token_type(&stream, TOKEN_OPEN_CURLY)) {
							token = try_merge;
							break;
						} else if (match_token_type(&stream, TOKEN_NEWLINE)) {
							break;
						} else {
							try_merge = token_union(try_merge, next_token(&stream));
						}
					}
					const char *block_type = NULL;
					while (!block_type) {
						if ((TOP(states)&0xFF00) == ST_IF) block_type = "if";
						if ((TOP(states)&0xFF00) == ST_WHILE) block_type = "while";
					}
					set_error(&ctx, &token, "The condition of an `$` statement must be "
							"enclosed in parentheses.",block_type);
					return NULL;
				}
				ADD(vm, states, ST_CONDITION | SU_FINISH);
				ADD(vm, states, ST_EXPRESSION);
			} break;
			case ST_CONDITION | SU_FINISH: {
				if (!match_token_type(&stream, TOKEN_CLOSE_PAREN)) {
					const char *block_type = NULL;
					while (!block_type) {
						if ((TOP(states)&0xFF00) == ST_IF) block_type = "if";
						if ((TOP(states)&0xFF00) == ST_WHILE) block_type = "while";
					}
					set_error(&ctx, &token, "A close-paren is missing here to enclose the "
							"condition of the `$` statement.", block_type);
					return NULL;
				}
			} break;
			case ST_EXPRESSION: {
				ADD(vm, states, ST_EXPRESSION | SU_MONO);
			} break;
			case ST_EXPRESSION | SU_MONO: {
				if (token.type == TOKEN_DEC_NUMBER) {
					(void)next_token(&stream);
				} else {
					set_error(&ctx, &token, "A value is expected here.");
					return NULL;
				}
				if (in_group(peek_token(&stream), group_binary)) {
					ADD(vm, states, ST_EXPRESSION | SU_INTER);
				}
			} break;
			case ST_EXPRESSION | SU_INTER: {
				(void)next_token(&stream);
				ADD(vm, states, ST_EXPRESSION | SU_MONO);
			} break;
			default: { 
				printf("state = %x\n",state);
				set_error(&ctx, &token, "An invalid state occurred in the compiler.\n\n"
						"This is a bug in Cardinal. Please report it to the devs.");
				return NULL;
			} break;
		}
		state_continue: {}
	}

	code->required_stack_space = stack_values_max;
	set_error(&ctx, NULL, "Compiled successfully!\n");

	return NULL;
}

