/* wait.c - wait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wait  -  Cause current process to wait on a semaphore
 *------------------------------------------------------------------------
 */
syscall	wait(
	  sid32		sem		/* Semaphore on which to wait  */
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	struct	sentry *semptr;		/* Ptr to sempahore table entry	*/
	
	struct procent *currptr;
	
	uint64_t start, end;
	
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	
	mask = disable();
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%eax", "%edx");
	currptr=&proctab[currpid];
	
	(currptr->frequency[5])+=1;
	if (isbadsem(sem)) {
		restore(mask);
		return SYSERR;
	}

	semptr = &semtab[sem];
	if (semptr->sstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--(semptr->scount) < 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = sem;		/* Record semaphore ID	*/
		enqueue(currpid,semptr->squeue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	}
	asm volatile ("RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
		"%eax", "%edx");
	start = ( ((uint64_t)cycles_high << 32) | cycles_low );
	
	end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );

	currptr->clockCycles[5]+=(end-start);
	restore(mask);
	return OK;
}
