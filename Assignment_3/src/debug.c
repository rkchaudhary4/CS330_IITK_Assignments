#include <debug.h>
#include <context.h>
#include <entry.h>
#include <lib.h>
#include <memory.h>

/*****************************HELPERS******************************************/

/* 
 * allocate the struct which contains information about debugger
 *
 */
struct debug_info *alloc_debug_info()
{
	struct debug_info *info = (struct debug_info *)os_alloc(sizeof(struct debug_info));
	if (info)
		bzero((char *)info, sizeof(struct debug_info));
	return info;
}

/*
 * frees a debug_info struct 
 */
void free_debug_info(struct debug_info *ptr)
{
	if (ptr)
		os_free((void *)ptr, sizeof(struct debug_info));
}

/*
 * allocates memory to store registers structure
 */
struct registers *alloc_regs()
{
	struct registers *info = (struct registers *)os_alloc(sizeof(struct registers));
	if (info)
		bzero((char *)info, sizeof(struct registers));
	return info;
}

/*
 * frees an allocated registers struct
 */
void free_regs(struct registers *ptr)
{
	if (ptr)
		os_free((void *)ptr, sizeof(struct registers));
}

/* 
 * allocate a node for breakpoint list 
 * which contains information about breakpoint
 */
struct breakpoint_info *alloc_breakpoint_info()
{
	struct breakpoint_info *info = (struct breakpoint_info *)os_alloc(
		sizeof(struct breakpoint_info));
	if (info)
		bzero((char *)info, sizeof(struct breakpoint_info));
	return info;
}

/*
 * frees a node of breakpoint list
 */
void free_breakpoint_info(struct breakpoint_info *ptr)
{
	if (ptr)
		os_free((void *)ptr, sizeof(struct breakpoint_info));
}

/*
 * Fork handler.
 * The child context doesnt need the debug info
 * Set it to NULL
 * The child must go to sleep( ie move to WAIT state)
 * It will be made ready when the debugger calls wait_and_continue
 */
void debugger_on_fork(struct exec_context *child_ctx)
{
	child_ctx->dbg = NULL;
	child_ctx->state = WAITING;
}

/******************************************************************************/

/* This is the int 0x3 handler
 * Hit from the childs context
 */
long int3_handler(struct exec_context *ctx)
{
	struct exec_context *p = get_ctx_by_pid(ctx->ppid);
	if (p == NULL)
		return -1;
	if (*((u8 *)(ctx->regs.entry_rip - 2)) == PUSHRBP_OPCODE)
	{
		*((u8 *)ctx->regs.entry_rip - 2) = INT3_OPCODE;
		*((u8 *)ctx->regs.entry_rip - 1) = p->dbg->next;
		--ctx->regs.entry_rip;
		schedule(ctx);
	}
	else
	{
		for (int i = 0; i < MAX_BACKTRACE; i++)
		{
			p->dbg->backtrace[i] = 0;
		}
		p->dbg->backtrace[0] = ctx->regs.entry_rip - 1;
		p->dbg->backtrace[1] = *((u64 *)ctx->regs.entry_rsp);
		int index = 2;
		u64 curr_rbp = ctx->regs.rbp;
		while (*(u64 *)(curr_rbp + 8) != END_ADDR)
		{
			p->dbg->backtrace[index++] = *(u64 *)(curr_rbp + 8);
			curr_rbp = *((u64 *)curr_rbp);
		}
		p->dbg->next = *((u8 *)ctx->regs.entry_rip);
		*((u8 *)ctx->regs.entry_rip) = INT3_OPCODE;
		p->regs.rax = --ctx->regs.entry_rip;
		*((u8 *)ctx->regs.entry_rip) = PUSHRBP_OPCODE;
		ctx->state = WAITING;
		p->state = READY;
		schedule(p);
	}
	return 0;
}

/*
 * Exit handler.
 * Called on exit of Debugger and Debuggee 
 */
void debugger_on_exit(struct exec_context *ctx)
{
	if (ctx->dbg == NULL)
	{
		get_ctx_by_pid(ctx->ppid)->state = READY;
		get_ctx_by_pid(ctx->ppid)->regs.rax = CHILD_EXIT;
	}
	else
	{
		struct debug_info *d = ctx->dbg;
		struct breakpoint_info *head = d->head;
		struct breakpoint_info *curr = head;
		while (head->next != NULL)
		{
			head = head->next;
			free_breakpoint_info(curr);
			curr = head;
		}
		free_breakpoint_info(curr);
		free_debug_info(d);
	}
}

/*
 * called from debuggers context
 * initializes debugger state
 */
int do_become_debugger(struct exec_context *ctx)
{
	ctx->dbg = alloc_debug_info();
	if (ctx->dbg == NULL)
		return -1;
	ctx->dbg->head = NULL;
	ctx->dbg->num = 0;
	for (int i = 0; i < MAX_BACKTRACE; i++)
	{
		ctx->dbg->backtrace[i] = 0;
	}
	return 0;
}

/*
 * called from debuggers context
 */
int do_set_breakpoint(struct exec_context *ctx, void *addr)
{
	struct breakpoint_info *head = ctx->dbg->head;
	*((u8 *)addr) = INT3_OPCODE;
	if (head == NULL)
	{
		head = alloc_breakpoint_info();
		if (head == NULL)
			return -1;
		head->addr = (u64)addr;
		head->next = NULL;
		head->num = ++ctx->dbg->num;
		head->status = 1;
		ctx->dbg->head = head;
		return 0;
	}
	else
	{
		int num = 1;
		while (head->next != NULL)
		{
			if (head->addr == (u64)addr)
			{
				head->status = 1;
				return 0;
			}
			num++;
			head = head->next;
		}
		if (head->addr == (u64)addr)
		{
			head->status = 1;
			return 0;
		}
		if (num >= MAX_BREAKPOINTS)
			return -1;
		head->next = alloc_breakpoint_info();
		if (head->next == NULL)
			return -1;
		head->next->num = ++ctx->dbg->num;
		head = head->next;
		head->addr = (u64)addr;
		head->next = NULL;
		head->status = 1;
		head = ctx->dbg->head;
		return 0;
	}
}

/*
 * called from debuggers context
 */
int do_remove_breakpoint(struct exec_context *ctx, void *addr)
{
	struct breakpoint_info *head = ctx->dbg->head;
	if (head->addr == (u64)addr)
	{
		ctx->dbg->head = head->next;
		*((u8 *)addr) = PUSHRBP_OPCODE;
		free_breakpoint_info(head);
		return 0;
	}
	else
	{
		while (head->next != NULL && head->next->addr != (u64)addr)
			head = head->next;
		if (head->next == NULL || head->next->addr != (u64)addr)
			return -1;
		struct breakpoint_info *to_delete = head->next;
		head->next = head->next->next;
		free_breakpoint_info(to_delete);
		*((u8 *)addr) = PUSHRBP_OPCODE;
		return 0;
	}
	return -1;
}

/*
 * called from debuggers context
 */
int do_enable_breakpoint(struct exec_context *ctx, void *addr)
{
	struct breakpoint_info *head = ctx->dbg->head;
	while (head != NULL && head->addr != (u64)addr)
	{
		head = head->next;
	}
	if (head == NULL)
		return -1;
	head->status = 0;
	*((u8 *)addr) = INT3_OPCODE;
	return 0;
}

/*
 * called from debuggers context
 */
int do_disable_breakpoint(struct exec_context *ctx, void *addr)
{
	struct breakpoint_info *head = ctx->dbg->head;
	while (head != NULL && head->addr != (u64)addr)
	{
		head = head->next;
	}
	if (head == NULL)
		return -1;
	head->status = 0;
	*((u8 *)addr) = PUSHRBP_OPCODE;
	return 0;
}

/*
 * called from debuggers context
 */
int do_info_breakpoints(struct exec_context *ctx, struct breakpoint *ubp)
{
	int active = 0;
	struct breakpoint_info *head = ctx->dbg->head;
	int index = 0;
	while (head != NULL)
	{
		ubp[index].addr = head->addr;
		ubp[index].num = head->num;
		ubp[index].status = head->status;
		index++;
		active++;
		head = head->next;
	}
	return active;
}

/*
 * called from debuggers context
 */
int do_info_registers(struct exec_context *ctx, struct registers *regs)
{
	struct exec_context *next;
	int pid = (ctx->pid + 1) == MAX_PROCESSES ? 1 : (ctx->pid + 1);
	while (pid)
	{
		struct exec_context *new_ctx = get_ctx_by_pid(pid);
		if (new_ctx->ppid == ctx->pid)
		{
			next = new_ctx;
			break;
		}
		++pid;
		if (pid == ctx->pid + 1)
			pid = 0;
	}
	if (pid == 0)
		return -1;
	regs->entry_rip = next->regs.entry_rip;
	regs->entry_rsp = next->regs.entry_rsp;
	regs->rbp = next->regs.rbp;
	regs->rax = next->regs.rax;
	regs->rdi = next->regs.rdi;
	regs->rsi = next->regs.rsi;
	regs->rdx = next->regs.rdx;
	regs->rcx = next->regs.rcx;
	regs->r8 = next->regs.r8;
	regs->r9 = next->regs.r9;
	return 0;
}

/* 
 * Called from debuggers context
 */
int do_backtrace(struct exec_context *ctx, u64 bt_buf)
{
	struct exec_context *next;
	u64 *arr = (u64 *)bt_buf;
	int index = 0;
	while (ctx->dbg->backtrace[index] != 0)
	{
		arr[index] = ctx->dbg->backtrace[index];
		index++;
	}
	return index;
}

/*
 * When the debugger calls wait
 * it must move to WAITING state 
 * and its child must move to READY state
 */

s64 do_wait_and_continue(struct exec_context *ctx)
{
	struct exec_context *next;
	int pid = (ctx->pid + 1) == MAX_PROCESSES ? 1 : (ctx->pid + 1);
	while (pid)
	{
		struct exec_context *new_ctx = get_ctx_by_pid(pid);
		if (new_ctx->ppid == ctx->pid)
		{
			next = new_ctx;
			break;
		}
		++pid;
		if (pid == ctx->pid + 1)
			pid = 0;
	}
	ctx->state = WAITING;
	next->state = READY;
	schedule(next);
	return -1;
}