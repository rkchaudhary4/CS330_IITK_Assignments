#include<ulib.h>



int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

	int pageSize = 4096;
	u32 MB = 1 << 20;
	// allocate 8M region
	char *addr = mmap(NULL, 8*MB, PROT_READ|PROT_WRITE, 0);
	pmap(1);
	printf("mmap start_addr = %x\n", addr); // addr is not 2MB aligned
	
	// find next 2M aligned 
	u64 aligned = (u64)addr;
	if(aligned & 0x1fffff){
		aligned >>= 21;
		++aligned;
		aligned <<= 21;
	}
	
	for (int i = 0; i < 4096 ; i++) {
		*(((char *)aligned) + i) = 'X';
	}

    char *addr_aligned = (char *) make_hugepage(addr, 8*MB , PROT_READ|PROT_WRITE, 0);
    /*
	  Find next huge page boundary address (aligned)
      Make as many huge pages as possible in the address range (aligned, addr+8M)
      The starting of the huge page should be from a 2MB aligned address, aligned
    */

    pmap(1);

    int diff = 0;
	for (int i = 0; i < 4096; i++) {
        // The content of the pages that get converted to huge pages are preserved.
		if (addr_aligned[i] != 'X') {
			++diff;	
		}
	}

    // printf("diff = %x\n", diff);
	if(diff)
		printf("Test failed\n");
	else
		printf("Test passed\n");

}

