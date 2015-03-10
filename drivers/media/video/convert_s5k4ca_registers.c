#include "s5k4ca.h"


/*
 * first you need to change in s5k4ca.h to convert s5k4ca_init0 and s5k4ca_init1
 * arrays from s5k4ca_short_t to "unsigned long"
 *
 * open s5k4ca.h and use these vim commands to do so
 *
 * :%s/, 0x//g
 * :%s/},/,/g
 * :%s/{0x/0x/g
 * 
 * after the modification run this program by
 * $convert > s5k4ca_init.h
 */ 
main()
{
	int i;

	printf("static unsigned long s5k4ca_init0[] = {\n");
	for(i=0; i<S5K4CA_INIT0_REGS; i++) { 
		printf("0x%x,\n", htonl(s5k4ca_init0[i]));
	}
	printf("};");

	printf("\n\n");

	printf("static unsigned long s5k4ca_init1[] = {\n");
	for(i=0; i<S5K4CA_INIT1_REGS; i++) { 
		printf("0x%x,\n", htonl(s5k4ca_init1[i]));
	}
	printf("};");
}
