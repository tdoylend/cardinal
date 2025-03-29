#include <stdio.h>
#include <stdlib.h>

#include <cardinal.h>
//#include "source/list.h"
//#include "source/vm.h"

int main(int argc, char *argv[]) {
	printf("[Program Start]\n");

	{
		Car_Version comp = CAR_COMPILED_VERSION;
		Car_Version link = car_get_linked_version();
		printf("Compiled version: %d.%d.%d\n",comp.major,comp.minor,comp.patch);
		printf("  Linked version: %d.%d.%d\n",link.major,link.minor,link.patch);
	}

	Car_Config config;
	if (!car_init_default_config(&config, NULL)) {
		fprintf(stderr,"Your version of Cardinal was compiled without default config.\n");
		fprintf(stderr,"Please recompile with a compatible version of Cardinal.\n");
		exit(2);
	}
	
	if (argc != 2) {
		fprintf(stderr,"You must supply the name of exactly one file to run as a script.\n");
		return 1;
	}
	FILE *f = fopen(argv[1],"rb");
	if (!f) {
		fprintf(stderr,"Could not open the script `%s`.\n",argv[1]);
		return 2;
	}
	fseek(f,0,SEEK_END);
	int length = ftell(f);
	fseek(f,0,SEEK_SET);
	char *source = malloc(length + 1);
	fread(source, length, 1, f);
	source[length] = 0;
	fclose(f);

	Car_VM *vm = car_new_vm(&config);

	Car_Interpret_Result result = car_interpret(vm, "main", argv[1], source);

	if (result == CAR_SUCCESS) {
		printf("Success!\n");
	} else if (result == CAR_COMPILATION_ERROR) {
		printf("Compilation error\n");
	} else if (result == CAR_RUNTIME_ERROR) {
		printf("Runtime error\n");
	} else {
		printf("Unknown result code?? How did we get here\n");
	}

	/*
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
	*/
	printf("[Program Complete]\n");
	return 0;
}
