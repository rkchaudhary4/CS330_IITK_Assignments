#include<ulib.h>

// Page fault handler working correctly.

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int pageSize = 4096;
	u32 MB = 1 << 20;

  u64 aligned = 0x180400000; // 2MB aligned address

	char *paddr = mmap((void *)aligned, 2*MB, PROT_READ|PROT_WRITE, 0);
  pmap(1);

	char *hpaddr = (char *) make_hugepage(paddr, 2*MB , PROT_READ|PROT_WRITE, 0);
	pmap(1);

  int ret = break_hugepage(hpaddr, 2*MB);
  pmap(1);

  if (ret < 0) {
    printf("Test failed");
  }

}