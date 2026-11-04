
/* v1495firmware.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "jvme.h"
#include "v1495.h"

#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int res, page = -1, user_vme = -1;
  char myname[256];
  unsigned int addr;
  char *laddr;

  if(argc==5)
  {
    addr = strtol(argv[1], (char **)NULL, 16);
    printf("use argument >0x%08x< as board VME address\n",addr);

    strncpy(myname, argv[2], 255);
    printf("use argument >%s< as rbf file name\n",myname);
	 
	 page = atoi(argv[3]);
	 user_vme = atoi(argv[4]);
  }
  else
  {
    printf("Usage: v1495firmware <vme_address> <rbf file> <page> <user_vme>\n");
	 printf("       page:     0=standard, 1=backup\n");
	 printf("       user_vme: 0=USER FPGA, 1=VME FPGA\n");
    exit(0);
  }

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  /* get address in A24 space */
  res = vmeBusToLocalAdrs(0x39, (char *)(uintptr_t)addr, &laddr);
  if (res != 0) 
  {
	printf("ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
  }
  else
  {
    printf("INFO: addr=0x%08x, laddr=%p\n", addr, (void *)laddr);
    if((uintptr_t)laddr > UINT_MAX)
    {
      fprintf(stderr, "ERROR: local VME address does not fit in this driver's 32-bit address type\n");
      exit(1);
    }
    /* update firmware */
    v1495firmware((unsigned int)(uintptr_t)laddr, myname, page, user_vme);
  }

  exit(0);
}

#else

int
main()
{
  return(0);
}

#endif
