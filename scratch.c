
void report(Car_VM *vm, Car_Report_Flags flags, Report_Line *lines, int count);














================================


			case PARSE_BLOCK: {
				if (match_token_type(&stream, TOKEN_NEWLINE)) {
					// it's an actual block, of statements
					TOP(state).subtype = STATEMENT_BLOCK;
					TOP(state).stage = STAGE_PENDING_BODY;
					TOP(state) = PARSE_STATEMENT_BLOCK_POST;
				} else {
					TOP(state) = PARSE_EXPRESSION_BLOCK_POST;
					ADD(vm, state, PARSE_EXPRESSION);
					ADD(vm, value_count, 0);
				}
			} break;
			case PARSE_STATEMENT_BLOCK_POST: {
				Token token = next_token(&stream);
				if (token.type == TOKEN_CLOSE_CURLY) {
					POP(state);
				} else {
					ADD(vm, state, PARSE_STATEMENT);
				}
			} break;
			case PARSE_EXPRESSION_BLOCK_POST: {
				Token token = next_token(&stream);
				if (token.type == TOKEN_CLOSE_CURLY) {
					POP(state);
				} else {
					set_error(&ctx, &token, "A closing curly brace `}` is expected at the end of"
							" a block.");
					return NULL;
				}
			} break;
			case PARSE_IF_STATEMENT_POST: {
				POP(state);
			} break;
			case PARSE_STATEMENT: {
				printf("statement %s\n", token_name(peek_token(&stream)));
				if (peek_token(&stream).type == TOKEN_UNKNOWN) {
					return display_unknown_token_error(&ctx);
				} else if (match_token_type(&stream, TOKEN_IF)) {
					if (!match_token_type(&stream, TOKEN_OPEN_PAREN)) {
						Token token = peek_token(&stream);
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
						set_error(&ctx, &token, "The condition of an `if` statement must be "
								"enclosed in parentheses.");
						return NULL;
					}
					TOP(state) = PARSE_IF_STATEMENT_BETWEEN_CONDITION_AND_BODY;
					ADD(vm, state, PARSE_EXPRESSION);
					ADD(vm, value_count, 0);
				} else {
					TOP(state) = PARSE_STATEMENT_AFTER;
					ADD(vm, state, PARSE_EXPRESSION);
					ADD(vm, value_count, 0);
				}
			} break;
			case PARSE_IF_STATEMENT_BETWEEN_CONDITION_AND_BODY: {
				if (!match_token_type(&stream, TOKEN_CLOSE_PAREN)) {
					Token token = peek_token(&stream);
					if (token.type == TOKEN_OPEN_CURLY) {
						token.end = token.start;
						token.start = token.start - 1;
					}
					set_error(&ctx, &token, "A closing parenthesis is missing.");
					return NULL;
				}
				TOP(state) = PARSE_IF_STATEMENT_POST;
				if (match_token_type(&stream, TOKEN_OPEN_CURLY)) {
					ADD(vm, state, PARSE_BLOCK);
				} else {
					ADD(vm, state, PARSE_STATEMENT);
				}
			} break;
			case PARSE_STATEMENT_AFTER: {
				Token token = next_token(&stream);
				if ((token.type == TOKEN_NEWLINE) || (token.type == TOKEN_EOF)) {
					(void)POP(state);
				} else if (in_group(token, expression_start)) {
					set_error(&ctx, &token, "An operator is missing to the left of this value.");
					return NULL;
				} else {
					set_error(&ctx, &token, "A newline (or EOF) is expected after a statement.");
					return NULL;
				}
			} break;
			case PARSE_EXPRESSION: {
				Token token = next_token(&stream);
				printf("expression %s\n", token_name(token));
				if (token.type == TOKEN_DEC_NUMBER) {
					char *end = &source->bytes[token.end];
					char *string = ALLOC(vm, 1+token.end-token.start);
					string[token.end-token.start] = 0;
					memcpy(string, &source->bytes[token.start], token.end-token.start);
					double value = strtod(string, NULL);
					//emit_constant(vm, code, (Value){.value = value});
					TOP(value_count) ++;
					TOP(state) = PARSE_EXPRESSION_INTER;
				} else if (in_group(token,group_prefix_unary)) {
					printf("unary op, you know how i be\n");
				} else if (in_group(token,group_binary)) {
					set_error(&ctx, &token,
							"This operator is binary but is missing a left-side operand.");
					return NULL;
				} else if (in_group(token,group_assignment)) {
					set_error(&ctx, &token,
							"A left-side variable is required into which to put the result.");
					return NULL;
				} else if (token.type == TOKEN_IDENTIFIER) {
					printf("unary value\n");
					TOP(value_count) ++;
					TOP(state) = PARSE_EXPRESSION_INTER;
				} else {
					set_error(&ctx, &token, "Expected a value here\n"); //@todo
					return NULL;
				}
			} break;
			case PARSE_EXPRESSION_INTER: {
				Token token = peek_token(&stream);
				printf("expression_inter %s\n", token_name(token));
				if (in_group(token, group_binary)) {
					(void)next_token(&stream);
					TOP(state) = PARSE_EXPRESSION;
				} else {
					POP(state);
				}
			} break;
