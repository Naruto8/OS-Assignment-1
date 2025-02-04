/* printtest.c
 *	Simple program to test whether printing from a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main()
{
	system_call_PrintString("hello world\n");
	system_call_PrintString("Executed ");
	system_call_PrintInt(system_call_GetTime());
	system_call_PrintInt(system_call_GetNumInstr());
	system_call_PrintString(" instructions.\n");
	return 0;
}
