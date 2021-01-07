#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i;
	if(argc == 1){
		// default mode
		for(i = 0; i < 10; ++i){
			printf("ECHO_DEFAULT_MODE_OUTPUT_LINE_%d\n", i);
		}
	}
	else if(argc == 2){
		if(strcmp(argv[1], "-a") == 0){
			for(i = 0; i < 10; ++i){
				printf("ECHO_MODE_A_OUTPUT_LINE_%d\n", i);
			}
		}
		else if(strcmp(argv[1], "-b") == 0){
			for(i = 0; i < 10; ++i){
				printf("ECHO_MODE_B_OUTPUT_LINE_%d\n", i);
			}
		}
		else if(strcmp(argv[1], "-c") == 0){
			for(i = 0; i < 10; ++i){
				printf("ECHO_MODE_C_OUTPUT_LINE_%d\n", i);
			}
		}
		else if(strcmp(argv[1], "-d") == 0){
			for(i = 0; i < 10; ++i){
				printf("ECHO_MODE_D_OUTPUT_LINE_%d\n", i);
			}
		}
		else if(strcmp(argv[1], "-e") == 0){
			for(i = 0; i < 10; ++i){
				printf("ECHO_MODE_E_OUTPUT_LINE_%d\n", i);
			}
		}
		else{
			printf("INVALID OPTION\n");
		}
	}
	else if(argc == 6){
		if(strcmp(argv[1], "-a") == 0 && 
		strcmp(argv[2], "-b") == 0 &&
		strcmp(argv[3], "-c") == 0 &&
		strcmp(argv[4], "-d") == 0 &&
		strcmp(argv[5], "-e") == 0 
		){
			for(i = 0; i < 10; ++i){
				printf("ALL_ARGUMENTS_MODE_OUTPUT_LINE_%d\n", i);
			}
		}
	}
	
	return 0;

}
