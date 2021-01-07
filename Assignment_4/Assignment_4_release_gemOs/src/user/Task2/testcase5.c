#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int pageSize = 4096;
	u32 MB = 1 << 20;
  u64 aligned = 0x180400000; // 2MB aligned address

	char *addr = mmap((void *)aligned, 2*MB, PROT_READ|PROT_WRITE, 0);
	pmap(1);
	printf("mmap start_addr = %x\n", addr);

  char *addr_aligned = (char *) make_hugepage(addr, 2*MB , PROT_READ|PROT_WRITE, 0);
  pmap(1);

  int ret = break_hugepage(addr_aligned + 33, 2*MB); // not page aligned
  if (!(ret < 0)) {
    printf("Failed\n"); 
  }

  ret = break_hugepage(addr_aligned, 10); // length < 2MB
  if (!(ret < 0)) {
    printf("Failed\n"); 
  }
	pmap(1);
}
