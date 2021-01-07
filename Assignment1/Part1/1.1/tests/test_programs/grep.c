#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i;
	if(argc == 1){
		// default mode
		for(i = 0; i < 10; ++i){
			printf("GREP_DEFAULT_MODE_OUTPUT_LINE_%d\n", i);
		}
	}
	else{
		if(strcmp(argv[1], "-a") == 0){
			for(i = 0; i < 10; ++i){
				printf("GREP_MODE_A_OUTPUT_LINE_%d\n", i);
			}
		}
		else if(strcmp(argv[1], "-b") == 0){
			for(i = 0; i < 10; ++i){
				printf("GREP_MODE_B_OUTPUT_LINE_%d\n", i);
			}
		}
		else{
			printf("INVALID OPTION\n");
		}
	}
	
	return 0;

}
