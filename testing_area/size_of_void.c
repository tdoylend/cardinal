#include <stdio.h>
#include <stddef.h>

typedef struct {} test_t;
#define unused __attribute((unused))

int main(unused int argc, unused char *argv[]) {
	printf("size of null = %d\n", (int)sizeof(test_t));
	return 0;
}
