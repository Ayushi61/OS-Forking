/* wakeup.c - wakeup */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */
void	wakeup(void)
{
	/* Awaken all processes that have no more time to sleep */
	struct procent *currptr;
	
	uint64_t start, end;
	
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%eax", "%edx");
	currptr=&proctab[currpid];
	
	(currptr->frequency[6])+=1;
	resched_cntl(DEFER_START);
	while (nonempty(sleepq) && (firstkey(sleepq) <= 0)) {
		ready(dequeue(sleepq));
	}

	resched_cntl(DEFER_STOP);
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
		"%eax", "%edx");
	start = ( ((uint64_t)cycles_high << 32) | cycles_low );
	
	end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );

	currptr->clockCycles[6]+=(end-start);
	return;
}
