
#include<ulib.h>

// Page fault handler working correctly.

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int pageSize = 4096;
	u32 MB = 1 << 20;

    u64 aligned = 0x180400000; // 2MB aligned address

	char *paddr = mmap((void *)aligned, 2*MB, PROT_READ|PROT_WRITE, 0);
    pmap(1);

    for (int i = 0; i < 4096 ; i++) {
		paddr[i] = 'X';
	}

	char *hpaddr = (char *) make_hugepage(paddr, 2*MB , PROT_READ|PROT_WRITE, 0);
	pmap(1);
    
    int diff = 0;
	for (int i = 0; i < 4096; i++) {
        // The content of the pages that get converted to huge pages are preserved.
		if (hpaddr[i] != 'X') {
			++diff;	
		}
	}
	
	if(diff)
		printf("FAILED\n");
	else
		printf("PASSED\n");

}