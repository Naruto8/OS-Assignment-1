#include "syscall.h"

int
main()
{
    int x, i;

    x = system_call_Fork();
   if(x == 0){
	for (i=0; i<5; i++) {
       system_call_PrintString("*** thread ");
    	system_call_PrintInt(system_call_GetPID());
       system_call_PrintString(" looped ");
       system_call_PrintInt(i);
       system_call_PrintString(" times.\n");
	system_call_Sleep(10);
       //system_call_Yield();
    }}
     else{
 //     system_call_Join(1);
  system_call_PrintString("Before join.\n");
            system_call_PrintString("After join.\n");
//	while(1);
    }
   system_call_Exit(0);
    return 0;
}
