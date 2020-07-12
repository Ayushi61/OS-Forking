/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{	
	struct procent *currptr;
	
	uint64_t start, end;
	
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	
	

	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%eax", "%edx");
	currptr=&proctab[currpid];
	
	(currptr->frequency[2])+=1;
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	insert(pid, readylist, prptr->prprio);
	resched();
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
		"%eax", "%edx");
	start = ( ((uint64_t)cycles_high << 32) | cycles_low );
	
	end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );

	currptr->clockCycles[2]+=(end-start);
	return OK;
}
