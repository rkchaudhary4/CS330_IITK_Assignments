#include<entry.h>
#include<lib.h>
#include<context.h>
#include<memory.h>
#include<schedule.h>
#include<file.h>
#include<kbd.h>
#include<page.h>
#include<mmap.h>

long do_fork()
{
	struct exec_context *new_ctx = get_new_ctx();
	struct exec_context *ctx = get_current_ctx();
	u32 pid = new_ctx->pid;

	*new_ctx = *ctx;  //Copy the process
	new_ctx->pid = pid;
	new_ctx->ppid = ctx->pid; 
	copy_mm(new_ctx, ctx);
	setup_child_context(new_ctx);
	return pid;
}


void do_exit() 
{
	/*You may need to invoke the scheduler from here if there are
	other processes except swapper in the system. Make sure you make 
	the status of the current process to UNUSED before scheduling 
	the next process. If the only process alive in system is swapper, 
	invoke do_cleanup() to shutdown gem5 (by crashing it, huh!)
	*/
	// Scheduling new process (swapper if no other available)
	int ctr;
	struct exec_context *ctx = get_current_ctx();
	struct exec_context *new_ctx;

	do_file_exit(ctx);
	
	// cleanup of this process
	os_pfn_free(OS_PT_REG, ctx->os_stack_pfn);
	ctx->state = UNUSED;
	// check if we need to do cleanup
	int proc_exist = -1;

	for(ctr = 1; ctr < MAX_PROCESSES; ctr++) {
		struct exec_context *new_ctx = get_ctx_by_pid(ctr);
		if(new_ctx->state != UNUSED) {
			proc_exist = 1;
			break;
		}
	}

	stats->num_processes--; 
	if(proc_exist == -1) 
		do_cleanup();  /*Call this conditionally, see comments above*/

	new_ctx = pick_next_context(ctx);
	schedule(new_ctx);  //Calling from exit
}

int do_file_open(struct exec_context *ctx,u64 filename, u64 flag, u64 mode)
{

	struct file *filep;
	struct inode * inode;
	u32 fd, i;

	dprintk("%s %u %u\n",filename,mode,flag);
	/*This code is to handle stdin,stdout,stderr cases */

	if(!strcmp((char*)filename,"stdin"))
		return open_standard_IO(ctx, STDIN);

	if(!strcmp((char*)filename,"stdout"))
		return open_standard_IO(ctx, STDOUT);

	if(!strcmp((char*)filename,"stderr"))
		return open_standard_IO(ctx, STDERR);

	/* END of stdin,stdout,stderr cases */
	
	// there is no regular file support
	return -1;
}

/* system call handler to read file 
 * no regular file support
 */

int do_file_read(struct exec_context *ctx, u64 fd, u64 buff, u64 count){
	int read_size = 0;
	struct file *filep = ctx->files[fd];
	dprintk("fd in read:%d\n",fd);


	if(!filep){
		return -1; //file is not opened
	}
	if((filep->mode & O_READ) != O_READ){
		return -EACCES; //file is write only
	}


	if(filep->fops->read){
		read_size = filep->fops->read(filep, (char*)buff, count);
		dprintk("buff inside read:%s\n",buff);
		dprintk("read size:%d\n",read_size);
		return read_size;
	}
	return -1;
}

/* system call handler to write file 
 * no regular file support
 */

int do_file_write(struct exec_context *ctx,u64 fd,u64 buff,u64 count){
	int write_size;
	struct file *filep = ctx->files[fd];
	if(!filep){
		return -1; //file is not opened
	}
	if(!(filep->mode & O_WRITE)){
		return -EACCES; // file is not opened in write mode
	}
	if(filep->fops->write){
		write_size = filep->fops->write(filep, (char*)buff, count);
		dprintk("write size:%d\n",write_size);
		return write_size;
	}
	return -1;
}

int do_close(struct exec_context *ctx, int fd)
{
	int ret;
	struct file *filep = ctx->files[fd];
	if(!filep || !filep->fops || !filep->fops->close){
		return -1; //file is not opened
	}
	ctx->files[fd] = NULL;
	ret = filep->fops->close(filep);
}

/*
 * Helper function to install a page table entry, given an address
 */
void install_page_table(struct exec_context *ctx, u64 addr, u64 error_code) {
	// get base addr of pgdir
	u64 *vaddr_base = (u64 *)osmap(ctx->pgd);
	u64 *entry;
	u64 pfn;
	// set User and Present flags
	// set Write flag if specified in error_code
	u64 ac_flags = 0x5 | (error_code & 0x2);
	
	// find the entry in page directory
	entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
	if(*entry & 0x1) {
		// PGD->PUD Present, access it
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		vaddr_base = (u64 *)osmap(pfn);
	}else{
		// allocate PUD
		pfn = os_pfn_alloc(OS_PT_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
		vaddr_base = osmap(pfn);
	}

	entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);
	if(*entry & 0x1) {
		// PUD->PMD Present, access it
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		vaddr_base = (u64 *)osmap(pfn);
	}else{
		// allocate PMD
		pfn = os_pfn_alloc(OS_PT_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
		vaddr_base = osmap(pfn);
	}

	entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
	if(*entry & 0x1) {
		// PMD->PTE Present, access it
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		vaddr_base = (u64 *)osmap(pfn);
	}else{
		// allocate PLD 
		pfn = os_pfn_alloc(OS_PT_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
		vaddr_base = osmap(pfn);
	}

	entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
	// since this fault occured as frame was not present, we don't need present check here
	pfn = os_pfn_alloc(USER_REG);
	*entry = (pfn << PTE_SHIFT) | ac_flags;
}

/*
 * Page fault handler
 */
int do_page_fault(struct user_regs *regs, u64 error_code)
{
	u64 rip, cr2;
	struct exec_context *current = get_current_ctx();
	rip = regs->entry_rip;
	stats->page_faults++;

	/*Get the Faulting VA from cr2 register*/
	asm volatile (
		"mov %%cr2, %0;"
		:"=r"(cr2)
		::"memory"
	);
	
	dprintk("PageFault:@ [RIP: %x] [accessed VA: %x] [error code: %x]\n", rip, cr2, error_code);
	
	if((error_code & 0x1)){
		dprintk("pid:%u\n",current->pid);
		dprintk("inside 0x1\n");
		goto sig_exit;
	} 

	if((cr2 >= (current -> mms)[MM_SEG_DATA].start) && (cr2 <= (current -> mms)[MM_SEG_DATA].end)){
		//Data segment
		dprintk("current pid:%d, error:%x \n",current->pid, error_code);
		if(cr2 < (current -> mms)[MM_SEG_DATA].next_free){
			// allocate
			install_page_table(current, cr2, error_code);
			goto done;
		}else{
			dprintk("Inside Data segment\n");
			goto sig_exit;
		}

	}

	if((cr2 >= (current -> mms)[MM_SEG_RODATA].start) && (cr2 <= (current -> mms)[MM_SEG_RODATA].end)) {
		// ROData Segment
		if(cr2 < (current -> mms)[MM_SEG_RODATA].next_free && ((error_code & 0x2) == 0)){
			//Bit pos 1 in error code = Read access
			install_page_table(current, cr2, error_code);
			goto done;
		}else{
			dprintk("Inside RO data\n");
			goto sig_exit;
		}
	}

	if ((cr2 >= (current -> mms)[MM_SEG_STACK].start) && (cr2 <= (current -> mms)[MM_SEG_STACK].end)) {
		// Stack Segment
		// If the page fault was caused by 
		// a stack segment page set the W bit 
		// in error code. 
		error_code |= 0x2;          

		if(cr2 < (current -> mms)[MM_SEG_STACK].next_free){
			cr2 = (cr2 >> PAGE_SHIFT) << PAGE_SHIFT;
			while((current -> mms)[MM_SEG_STACK].next_free != cr2){
				(current -> mms)[MM_SEG_STACK].next_free -= PAGE_SIZE;   
				install_page_table(current, (current -> mms)[MM_SEG_STACK].next_free, error_code);
				// This marks the usage range of stack  from stack end to stack next_free
			}
			install_page_table(current, (current -> mms)[MM_SEG_STACK].next_free, error_code);
			goto done;
		}else{
			dprintk("pid:%u\n",current->pid);
			dprintk("Inside Stack\n");
			goto sig_exit;
		}
	}

	if(cr2 >= MMAP_AREA_START && cr2 <= MMAP_AREA_END)
	{
		stats->mmap_page_faults++;
		int result = vm_area_pagefault(current, cr2, error_code);
		if(result > 0) 
			goto done;
		else{
			goto sig_exit;
		}
	}

sig_exit:
	printk("(Sig_Exit) PF Error @ [RIP: %x] [accessed VA: %x] [error code: %x]\n", rip, cr2, error_code);
	validate_page_table(current, cr2, 0);
	invoke_sync_signal(SIGSEGV, &regs->entry_rsp, &regs->entry_rip);
	return 0;

done:
	return 0;
}

/*System Call handler*/
long  do_syscall(int syscall, u64 param1, u64 param2, u64 param3, u64 param4)
{
	struct exec_context *current = get_current_ctx();
	unsigned long saved_sp;

	asm volatile(
		"mov %%rbp, %0;"
		: "=r" (saved_sp) 
		:
		: "memory"
	);  

	saved_sp += 0x10;    //rbp points to entry stack and the call-ret address is pushed onto the stack
	memcpy((char *)(&current->regs), (char *)saved_sp, sizeof(struct user_regs));  //user register state saved onto the regs 
	stats->syscalls++;
	dprintk("[GemOS] System call invoked. syscall no = %d\n", syscall);
	switch(syscall)
	{
	case SYSCALL_EXIT:
		dprintk("[GemOS] exit code = %d\n", (int) param1);
		do_exit();
		break;
	case SYSCALL_GETPID:
		dprintk("[GemOS] getpid called for process %s, with pid = %d\n", current->name, current->pid);
		return current->pid;      
	case SYSCALL_EXPAND:
		return do_expand(current, param1, param2);
	case SYSCALL_SHRINK:
		return do_shrink(current, param1, param2);
	case SYSCALL_ALARM:
		return do_alarm(param1);
	case SYSCALL_SLEEP:
		return do_sleep(param1);
	case SYSCALL_SIGNAL: 
		return do_signal(param1, param2);
	case SYSCALL_CLONE:
		return do_clone((void *)param1, (void *)param2);
	case SYSCALL_FORK:
		return do_fork();
	case SYSCALL_STATS:
		printk("ticks = %d swapper_invocations = %d context_switches = %d lw_context_switches = %d\n", 
		stats->ticks, stats->swapper_invocations, stats->context_switches, stats->lw_context_switches);
		printk("syscalls = %d page_faults = %d used_memory = %d num_processes = %d\n",
		stats->syscalls, stats->page_faults, stats->used_memory, stats->num_processes);
		printk("copy-on-write faults = %d allocated user_region_pages = %d\n",stats->cow_page_faults,
		stats->user_reg_pages);
		break;
	case SYSCALL_GET_USER_P:
		return stats->user_reg_pages;
	case SYSCALL_GET_COW_F:
		return stats->cow_page_faults;

	case SYSCALL_CONFIGURE:
		memcpy((char *)config, (char *)param1, sizeof(struct os_configs));      
		break;
	case SYSCALL_PHYS_INFO:
		printk("OS Data strutures:     0x800000 - 0x2000000\n");
		printk("Page table structures: 0x2000000 - 0x6400000\n");
		printk("User pages:            0x6400000 - 0x20000000\n");
		break;

	case SYSCALL_DUMP_PTT:
		return (u64) get_user_pte(current, param1, 1);

	case SYSCALL_MMAP:
		return (long) vm_area_map(current, param1, param2, param3, param4);

	case SYSCALL_MUNMAP:
		return (u64) vm_area_unmap(current, param1, param2);
	case SYSCALL_PMAP:
		return (long) vm_area_dump(current->vm_area, (int)param1);
	case SYSCALL_OPEN:
		return do_file_open(current,param1,param2,param3);
	case SYSCALL_READ:
		return do_file_read(current,param1,param2,param3);
	case SYSCALL_WRITE:
		return do_file_write(current,param1,param2,param3);
	case SYSCALL_CLOSE:
		return do_close(current, param1);
	case SYSCALL_MAKE_HUGEPAGE:
		return vm_area_make_hugepage(current, (void *)param1,(u32)param2, (u32)param3, (u32)param4);
	case SYSCALL_BREAK_HUGEPAGE:
		return vm_area_break_hugepage(current, (void *)param1, (u32)param2);
	default:
		return -1;
	}
	return 0;   /*GCC shut up!*/
}
