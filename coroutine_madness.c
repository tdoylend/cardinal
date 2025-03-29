#include <stdio.h>

#define STATE_MAIN	0
#define STATE_SUB	1

#define CR_CALL(substate)	\
	execution_point[state_count - 1] = -__LINE__; \
	state[state_count] = substate; \
	execution_point[state_count ++] = substate; \
	break; \
	case -__LINE__: while (0)

#define CR_RETURN()			(state_count --); break

void do_the_thing(void) {
	int state[128];
	int execution_point[128];
	int state_count = 0;

	state[0] = STATE_MAIN;
	execution_point[0] = STATE_MAIN;
	state_count = 1;

	while (state_count) {
		switch (execution_point[state_count - 1]) {
			case STATE_MAIN: {
				printf("Hello there\n");
				CR_CALL(STATE_SUB);
				printf("Back from the substate!\n");
				CR_CALL(STATE_SUB);
				CR_RETURN();
			};
			case STATE_SUB: {
				printf("This is a substate.\n");
				CR_RETURN();
			};
		}
	}
}

int main(int argc, char *argv[]) {
	printf("Hello world!\n\n");

	do_the_thing();

	printf("\nThing done!\n");
	return 0;
}
