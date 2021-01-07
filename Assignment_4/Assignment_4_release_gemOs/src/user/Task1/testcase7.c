#include<ulib.h>

// munmap with holes

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

	int pageSize = 4096;
	u32 M = 1 << 20;

    char *addr1 = mmap(NULL, pageSize, PROT_READ, 0);

    char *addr2 = mmap(addr1 + 2*pageSize, pageSize, PROT_READ, 0);

    pmap(1); // vm_area count 2;

    munmap(addr1, 3*pageSize);

    pmap(1); // vm_area count 0;


}
