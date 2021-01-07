#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  char * mm1 = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1
  pmap(0);

  for(int i = 0; i < 50; i++)
  {
      if(i % 2 != 0)
      {
          unsigned long addr = (unsigned long)mm1 + (pages * i);
          int val1 = munmap((void*)addr, pages);
          if(val1 < 0)
          {
            printf("Test case failed \n");
            return 1;
          }
      }
  }
  // vm_area count should be 25
  pmap(0);

  for(int i = 0; i < 50; i++)
  {
      if(i % 2 != 0)
      {
          char * mm2 = mmap(NULL, pages, PROT_READ|PROT_WRITE, 0);
          if((long)mm2 < 0)
          {
            printf("Test case failed \n");
            return 1;
          }
      }
  }
  // vm_area count should be 1;  
  pmap(0);

return 0;
}
