#include<ulib.h>

//Remove breakpoint that hasn't been set

int fn_3()
{
	printf("Inside function 3\n");
}

int fn_2()
{
	printf("Inside function 2\n");
}

int fn_1()
{
	printf("Inside function 1\n");
}

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int cpid;
	long ret = 0;
		
	ret = become_debugger();
	printf("become_debugger() = %x\n", ret);
	
	cpid = fork();
	
	if(cpid < 0){

	}
	else if(cpid == 0){
		printf("child : calling fn_1\n");
		fn_1();
		printf("child returned from fn_1\n");
		fn_2();
		fn_3();
		printf("child: Exiting\n");

	}
	else{
		printf("parent : setting breakpoint on fn_1: %x\n", fn_1);
		ret = set_breakpoint(fn_1);
		printf("parent : Return value of set_breakpoint(fn_1): %x\n", ret);

		printf("parent : calling wait_and_continue()\n");
		ret = wait_and_continue();
		printf("parent : Return value of wait_and_continue(): %x\n", ret);


		printf("parent : calling remove_breakpoint()\n");
		ret = remove_breakpoint(fn_2);
		printf("parent : Returned from remove_breakpoint(fn_2): %x\n", ret);

		
		
		printf("parent : calling wait_and_continue()\n");
		ret = wait_and_continue();
		printf("parent : Return value of wait_and_continue(): %x\n", ret);
	}
	
	return 0;
}
