#include <stdio.h>

#define BITS_PER_LONG 64
#define __BITS_PER_LONG 32

/*
 * FIXME: The check currently breaks x86-64 build, so it's
 * temporarily disabled. Please fix x86-64 and reenable
 */
#if 0 && BITS_PER_LONG != __BITS_PER_LONG
#define error 	1
#else
#define error 	0
#endif

void main(void)
{
	printf("\r\nHello Yotco! %d\n", error);
}
