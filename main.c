#include <stdio.h>
#include <stdlib.h>

#include <cardinal.h>
#include "source/list.h"
#include "source/vm.h"

int main(int argc, char *argv[]) {
	printf("[Program Start]\n");


	{
		Car_Version comp = CAR_COMPILED_VERSION;
		Car_Version link = car_get_linked_version();
		printf("Compiled version: %d.%d.%d\n",comp.major,comp.minor,comp.patch);
		printf("  Linked version: %d.%d.%d\n",link.major,link.minor,link.patch);
	}

	Car_Config config;
	if (!car_init_default_config(&config)) {
		fprintf(stderr,"Your version of Cardinal was compiled without default config.\n");
		fprintf(stderr,"Please recompile with a compatible version of Cardinal.\n");
		exit(2);
	}
	config.userdata = malloc(car_get_default_userdata_size());

	Car_VM *vm = car_new_vm(&config);

	printf("Your code here.\n");
	
	LIST(int, test);
	INIT_LIST(test);
	(void)ADD(vm, test, 5);
	for (size_t i = 0; i < COUNT(test); i ++) {
		printf("%d\n", test[i]);
	}

	FILE *f = fopen("script.car","rb");
	fseek(f,0,SEEK_END);
	int length = ftell(f);
	fseek(f,0,SEEK_SET);
	char *source = malloc(length+1);
	fread(source, length, 1, f);
	source[length] = 0;
	fclose(f);
	printf("<<<%s>>>\n",source);

	car_interpret(vm, "main", source);

	car_free_vm(vm);

	free(config.userdata);
	
	printf("[Program Complete]\n");
	return 0;
}
