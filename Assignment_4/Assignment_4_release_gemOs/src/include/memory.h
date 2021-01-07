#ifndef __MEMORY_H_
#define __MEMORY_H_
#include<types.h>

#define PAGE_SIZE 4096
#define HUGE_PAGE_SIZE (1<<21)
#define PAGE_SHIFT 12
#define HUGEPAGE_SHIFT 21 


#define PGD_MASK 0xff8000000000UL 
#define PUD_MASK 0x7fc0000000UL
#define PMD_MASK 0x3fe00000UL
#define PTE_MASK 0x1ff000UL

#define PGD_SHIFT 39
#define PUD_SHIFT 30
#define PMD_SHIFT 21
#define PTE_SHIFT 12

#define FLAG_MASK 0x3ffffffff000UL 

// This is physical memory ,not VAS
// 8 MB
#define REGION_OS_DS_START 0x800000
// 32 MB
#define REGION_OS_PT_START 0x2000000
// 100 MB
#define REGION_USER_START  0x6400000 
// 1 GB
#define REGION_HUGEPAGE_START  0x40000000
// 2 GB
#define ENDMEM                  0x80000000

/*XXX Total PMD entries (vmem = OS_PT_MAPS*2MB) USER_REGION bitmap is @ 100MB, so we must map atleast 128MB*/
#define OS_PT_MAPS 1024   

#define MAP_RD  0x0
#define MAP_WR  0x1

#define NUM_PAGES 0x80000

enum{
	OS_DS_REG,
	OS_PT_REG,
	USER_REG,
	HUGEPAGE_REG,
	MAX_REG
};

enum{
        OSALLOC_32,
        OSALLOC_64,
        OSALLOC_128,
        OSALLOC_256,
        OSALLOC_512,
        OSALLOC_1024,
        OSALLOC_2048,
        OSALLOC_MAX
};
struct page_list{
	u32 type;
	u32 size;
	u32 free_pages;
	u32 last_free_pos;
	u64 start_address;
	void *bitmap;
};

// accessed from hugepage.c and memory.c
extern struct page_list pglists[MAX_REG];

struct osalloc_chunk{
	u16 chunk_size;
	u16 free_position;
	u32 current_pfn;
	struct list freelist;
	char bitmap[16];   /*current page bitmap*/
}; 

#define NODE_MEM_PAGES 100

struct nodealloc_memory{
	u32 next_free;
	u32 num_free;
	void *nodes;
	void *end;
};

static inline int get_mem_region(u32 pfn)
{
	u64 address = (u64) pfn << PAGE_SHIFT;
	if(address < REGION_OS_DS_START || address > ENDMEM)
		return -1;
	else if (address >= REGION_OS_DS_START && address < REGION_OS_PT_START)
		return OS_DS_REG;
	else if (address >= REGION_OS_PT_START && address < REGION_USER_START)
		return OS_PT_REG;
	else if (address >= REGION_USER_START && address < REGION_HUGEPAGE_START)
		return USER_REG;
	return HUGEPAGE_REG;
}


extern void init_mems();
extern void* os_alloc(u32 size);
extern void os_free(void *, u32);
extern void* os_page_alloc(u32);
extern u32 os_pfn_alloc(u32);
extern void os_pfn_free(u32, u64);
extern void os_page_free(u32, void *);
extern struct node *node_alloc(void);
extern void node_free(struct node *);
extern int get_free_pages_region(u32);
void *osmap(u64);
extern void init_hugepages();

// hugepage helper functions 

// allocates a hugepage and gives starting address of the
// hugepage, this address can be used for copying data to 
// hugepage etc.
extern void *os_hugepage_alloc();

// given starting address of a hugepage, gives the frame no.
// of the hugepage
extern u64 get_hugepage_pfn(void *page_addr);

// free a hugepage
extern void os_hugepage_free(void *page_addr);
#endif
