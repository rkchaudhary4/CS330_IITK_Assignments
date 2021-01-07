#include<ulib.h>

int fn(int a, int b, int c, int d, int e, int f)
{
	printf("Inside child_fn3.\n");
	printf("Args of child_fn3: a: %x, b: %x, c: %x, d: %x, e: %x, f: %x\n", a, b, c, d, e, f);
}


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int cpid;
	long ret = 0;
	
	struct registers reg_info;			//registers info

	int bt_count = 0;		
	int bp_count = 0;			

	become_debugger();
	printf("User space: main() addr:%x\n", main);
	printf("User space: fn() addr:%x\n", fn);

	cpid = fork();
	
	if(cpid < 0){

	}
	else if(cpid == 0){
		fn(0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666);
		printf("Child returned from fn call. Exiting now.\n");
	}
	else{
		set_breakpoint(fn);
		
		// wait for breakpoint
		ret = wait_and_continue();
		printf("wait_and_continue() =  %x\n", ret);

		info_registers(&reg_info);	
		printf("Registers:\n");
		printf("r15: %x\n", reg_info.r15);
		printf("r14: %x\n", reg_info.r14);
		printf("r13: %x\n", reg_info.r13);
		printf("r12: %x\n", reg_info.r12);
		printf("r11: %x\n", reg_info.r11);
		printf("r10: %x\n", reg_info.r10);
		printf("r9: %x\n", reg_info.r9);
		printf("r8: %x\n", reg_info.r8);
		printf("rbp: %x\n", reg_info.rbp);
		printf("rdi: %x\n", reg_info.rdi);
		printf("rsi: %x\n", reg_info.rsi);
		printf("rdx: %x\n", reg_info.rdx);
		printf("rcx: %x\n", reg_info.rcx);
		printf("rbx: %x\n", reg_info.rbx);
		printf("rax: %x\n", reg_info.rax);
		printf("entry_rip: %x\n", reg_info.entry_rip);
		printf("entry_cs: %x\n", reg_info.entry_cs);
		printf("entry_rflags: %x\n", reg_info.entry_rflags);
		printf("entry_rsp: %x\n", reg_info.entry_rsp);
		printf("entry_ss: %x\n", reg_info.entry_ss);
		
		// wait for exit		
		ret = wait_and_continue();
		printf("wait_and_continue() = %x\n", ret);

	}
	
	return 0;
}
