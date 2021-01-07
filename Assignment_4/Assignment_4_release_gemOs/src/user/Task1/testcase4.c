#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 <= 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
    // vm_area count should be 1.
  pmap(0);


  // Hint address is free (not mapped). So it should create a vm_area with requested hint address and return it.
  char *hint_address = mm1 + (pages * 10);

  char *mm2  = mmap(hint_address, pages,PROT_READ|PROT_WRITE, 0);

  if(mm2 <= 0 || (long)hint_address != (long)mm2)
  {
    // Testcase failed
    printf("Test case failed \n");
    pmap(1);
    return 0;
  }
    // vm_area count should be 2.
  pmap(0);

  // Hint address is already mapped.  It should look 
  // for the next immediated free addresses in the address space 
  // which can serve the request and return it successfully.
  char *mm3  = mmap(mm1, pages,PROT_READ, 0);

  if(mm3 <= 0)
  {
    // Testcase failed
    printf("Test case failed \n");
    return 0;
  }
  
  // vm_area count should be 3.
  pmap(0);


  int munmap1 = munmap(mm3, pages);

  if(munmap1 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 2.
  pmap(0);


  int munmap2 = munmap(mm2, pages);

  if(munmap2 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 1.
  pmap(0);


  int munmap3 = munmap(mm1, pages*2);

  if(munmap3 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 0.
  pmap(0);

 return 0;
}
