/* yield.c - yield */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  yield  -  Voluntarily relinquish the CPU (end a timeslice)
 *------------------------------------------------------------------------
 */
syscall	yield(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct procent *currptr;
	
	uint64_t start, end;
	
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	
	mask = disable();
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%eax", "%edx");
	currptr=&proctab[currpid];
	
	(currptr->frequency[7])+=1;
	resched();
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
		"%eax", "%edx");
	start = ( ((uint64_t)cycles_high << 32) | cycles_low );
	
	end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );

	currptr->clockCycles[7]+=(end-start);
	restore(mask);
	return OK;
}
