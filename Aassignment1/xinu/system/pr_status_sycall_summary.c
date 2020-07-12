#include <xinu.h>

void pr_status_syscall_summary(){
	struct procent *prptr;
	uint32 i,k;
	bool flag=FALSE;
	char arr[][40]={"create","kill","ready","sleepms","suspend","wait","wakeup","yield"};
	

	kprintf("%3s %-7s %5s %14s\n",
		   "Pid", "syscall", "count", "average cycles");

	kprintf("%3s %-7s %5s %14s\n",
		   "---", "-------", "-----", "--------------");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		flag=FALSE;
		for(k=0;k<8;k++)
		{
		if(prptr->frequency[k]>0){
			printf("%3d %-7s %5d %14u\n",i,arr[k],prptr->frequency[k],(prptr->clockCycles[k])/(prptr->frequency[k]));
	
		
		flag=TRUE;
		}
		
		}
		if(flag==TRUE)
		kprintf("%3s %-7s %5s %14s\n",
		   "---", "-------", "-----", "--------------");
	}
	

	
}
