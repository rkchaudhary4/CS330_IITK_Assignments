#include<ulib.h>

int fn_3()
{
	printf("In fn3\n");	
	return 0;
}
int fn_2()
{
	printf("In fn2\n");	
	return 0;
}

int fn_1()
{
	printf("In fn1\n");	
	return 0;
}


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int cpid;
	long ret = 0;
	int i, bp_count;
	struct breakpoint bp_info[MAX_BREAKPOINTS];	//store breakpoints info here

	ret = become_debugger();
	
	cpid = fork();
	
	if(cpid < 0){
		printf("Error in fork\n");
	}
	else if(cpid == 0){
		fn_1();
		fn_2();
		fn_3();
		fn_2();
		printf("Child exited.\n");
	}
	else{
		ret = set_breakpoint(fn_1);
		ret = set_breakpoint(fn_2);
		ret = set_breakpoint(fn_3);

		printf("Address fn_1: %x\n", fn_1);
		printf("Address fn_2: %x\n", fn_2);
		printf("Address fn_3: %x\n", fn_3);
		
		// fn_1 
		ret = wait_and_continue();
		printf("Breakpoint occurred at address: %x\n", ret);

		disable_breakpoint(fn_2);

		// fn_3
		ret = wait_and_continue();
		printf("Breakpoint occurred at address: %x\n", ret);

		enable_breakpoint(fn_2);
		
		
		// fn_2
		ret = wait_and_continue();	
		printf("Breakpoint occurred at address: %x\n", ret);

		// for exit
		ret = wait_and_continue();	
		printf("Parent exited.\n");
	}
	
	return 0;
}
