#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

	int pageSize = 4096;
	u32 MB = 1 << 20;

  u64 aligned = 0x180400000; // 2MB aligned address

	char *addr1 = mmap((void *)aligned, 1*MB, PROT_READ, 0);
  char *addr2 = mmap((void *)addr1+(1*MB), 1*MB, PROT_WRITE, 0);
  char *addr3 = mmap((void *)addr2+(1*MB), 1*MB, PROT_READ, 0);
  char *addr4 = mmap((void *)addr3+(1*MB), 1*MB, PROT_WRITE, 0);

	pmap(1);

  long ret = make_hugepage(addr1, 4*MB , PROT_READ, 0);  // different permissions without force prot
  if (!(ret < 0)) {
    printf("Failed"); // should have returned error 
    return 1;
  }

  pmap(1);

  char* addr5 = (char*) make_hugepage(addr1, 4*MB , PROT_READ, 1); // different permissions with force prot
  pmap(1);
	
}
