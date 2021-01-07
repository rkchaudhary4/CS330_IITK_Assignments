#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(0);


  char *hint_address = mm1 + (pages * 10);
// Hint address is free (not mapped). So it should create a vm_area with requested hint address and return it.
  char *mm2  = mmap(hint_address, pages,PROT_READ|PROT_WRITE, MAP_FIXED);

  if((long)mm2 < 0 || (long)hint_address != (long)mm2)
  {
    // Testcase failed
    printf("Test case failed \n");
    pmap(1);
    return 0;
  }

  // vm_area count should be 2.
  pmap(0); 

  // Hint address is already mapped with some vm_area. So mmap should fail and return-1 because of MAP_fixed flag
  char *mm3  = mmap(mm1, pages,PROT_READ, MAP_FIXED);
  if((long)mm3 > 0)
  {
     // Testcase failed.
     printf("%x Test case failed %x \n", mm3, mm1);
     pmap(1);
     return 0;
  }
  // vm_area count should be 2.
  pmap(0); 
 return 0;
}
