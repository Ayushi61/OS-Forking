#include <xinu.h>

local	int newpid();	
pid32 fork(){
	/* Declaring variables to use in code */
	uint32		ssize;		/* Stack size in bytes		*/
	pri16	priority;	/* Process priority > 0		*/
	char		*name;		/* Name (for debugging)		*/
	uint32		savsp, *pushsp;	/* to save the stack pointers of the child*/
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	struct	procent *currptr;	/*parent child proc. table entry */
	int32		i,j;		/* for loop counter */
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/
	uint32		*psaddr;	/* parent stack address		*/
	uint32		total_offset;	/*offset of stackptr from the parent basp ptr*/
	unsigned long	*sp, *fp;	/* pointers for parent */
	uint32 offset,base_offset;	/*offsets to create child stack */
	unsigned long *fp_copy,addr_fp_copy;	/*copy of fp and address of fp */
	unsigned long *saddr_pre_copy;		/*coppy of saddr with value of stack base pointer */ 
	

	mask = disable();		/*disable interrupts */
	currptr=&proctab[currpid];	/* pcb pointer to the current running process*/
	ssize=currptr->prstklen;	/*copy the stack size of the parent process to the variable */
	priority=currptr->prprio;	/* copy priority of the parent to variable for child */
	name=currptr->prname;		/*copy the name of the parent to the variable for child */
		
	/*Get the stack pointer and function pointer of the parent */ 
	asm("movl %%esp, %0\n" :"=r"(sp));
	asm("movl %%ebp, %0\n" :"=r"(fp));
	/*initialize the stack size of the child process */
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (priority < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}
	/*copy contents from parent to child */
	prcount++;
	prptr = &proctab[pid];		/*pointer for child process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	prptr->prctr1000=ctr1000;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/* Initialize stack as if the process was called		*/
	*saddr = STACKMAGIC;


	/*caclulate offset from the base of the parent stach to the function pointer of the test frame */
	base_offset=(unsigned long *)currptr->prstkbase-fp;
	saddr_pre_copy=saddr;		/*save copy of the start of the child stack */
	saddr=saddr-base_offset;	/*calculate the fp and save it in saddr */
	unsigned long *saddr_copy=saddr;	/*copy of child fp in saddr_copy to later reach back to fp to fill up the registers for context switch*/
	
	savsp = saddr;		/*store the sp as the current address - fp for later restoring registers */
	fp_copy=fp;		/*copy of fp */
	
	/*copy elements in parents stack to child stack, and if it is fp, then modify the value of the stack w.r.t child stack. copy stack frames excluding fork frame */
	
	while(fp_copy!=(unsigned long *)currptr->prstkbase)
	{	
		
		addr_fp_copy=*fp_copy;
		offset=(unsigned long *)addr_fp_copy-fp_copy;
		*saddr=saddr+offset;
		
		for(j=0;j<offset;j++)
		{	
			*++saddr=*++fp_copy;
		}
		
	
	}
	/*restore back to fp of the child stack */
	saddr=saddr_copy;
	*(saddr_pre_copy-4)=prptr->prstkbase;
	
	/*registers for context switch */
	*--saddr = 0x00000200;		/*interrupt flags */
	*--saddr = NPROC;		/* %eax to return if it is child process */ 
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	
	
	restore(mask);
	resume(pid);
	
	return pid;
	
	
	

}


local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}
