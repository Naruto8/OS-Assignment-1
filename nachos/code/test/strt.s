# 1 "start.s"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "start.s"
# 10 "start.s"
# 1 "../userprog/syscall.h" 1
# 16 "../userprog/syscall.h"
# 1 "../threads/copyright.h" 1
# 17 "../userprog/syscall.h" 2
# 11 "start.s" 2

        .text
        .align 2
# 24 "start.s"
 .globl __start
 .ent __start
__start:
 jal main
 move $4,$0
 jal system_call_Exit
 .end __start
# 45 "start.s"
 .globl system_call_Halt
 .ent system_call_Halt
system_call_Halt:
 addiu $2,$0,0
 syscall
 j $31
 .end system_call_Halt

 .globl system_call_Exit
 .ent system_call_Exit
system_call_Exit:
 addiu $2,$0,1
 syscall
 j $31
 .end system_call_Exit

 .globl system_call_Exec
 .ent system_call_Exec
system_call_Exec:
 addiu $2,$0,2
 syscall
 j $31
 .end system_call_Exec

 .globl system_call_Join
 .ent system_call_Join
system_call_Join:
 addiu $2,$0,3
 syscall
 j $31
 .end system_call_Join

 .globl system_call_Create
 .ent system_call_Create
system_call_Create:
 addiu $2,$0,4
 syscall
 j $31
 .end system_call_Create

 .globl system_call_Open
 .ent system_call_Open
system_call_Open:
 addiu $2,$0,5
 syscall
 j $31
 .end system_call_Open

 .globl system_call_Read
 .ent system_call_Read
system_call_Read:
 addiu $2,$0,6
 syscall
 j $31
 .end system_call_Read

 .globl system_call_Write
 .ent system_call_Write
system_call_Write:
 addiu $2,$0,7
 syscall
 j $31
 .end system_call_Write

 .globl system_call_Close
 .ent system_call_Close
system_call_Close:
 addiu $2,$0,8
 syscall
 j $31
 .end system_call_Close

 .globl system_call_Fork
 .ent system_call_Fork
system_call_Fork:
 addiu $2,$0,9
 syscall
 j $31
 .end system_call_Fork

 .globl system_call_Yield
 .ent system_call_Yield
system_call_Yield:
 addiu $2,$0,10
 syscall
 j $31
 .end system_call_Yield

 .globl system_call_PrintInt
 .ent system_call_PrintInt
system_call_PrintInt:
        addiu $2,$0,11
        syscall
        j $31
 .end system_call_PrintInt

 .globl system_call_PrintChar
 .ent system_call_PrintChar
system_call_PrintChar:
        addiu $2,$0,12
        syscall
        j $31
 .end system_call_PrintChar

 .globl system_call_PrintString
 .ent system_call_PrintString
system_call_PrintString:
        addiu $2,$0,13
        syscall
        j $31
 .end system_call_PrintString

 .globl system_call_GetReg
 .ent system_callGetReg
system_call_GetReg:
 addiu $2,$0,14
 syscall
 j $31
 .end system_call_GetReg

 .globl system_call_GetPA
 .ent system_call_GetPA
system_call_GetPA:
 addiu $2,$0,15
 syscall
 j $31
 .end system_call_GetPA

 .globl system_call_GetPID
 .ent system_call_GetPID
system_call_GetPID:
 addiu $2,$0,16
 syscall
 j $31
 .end system_call_GetPID

 .globl system_call_GetPPID
 .ent system_call_GetPPID
system_call_GetPPID:
 addiu $2,$0,17
 syscall
 j $31
 .end system_call_GetPPID

 .globl system_call_Sleep
 .ent system_call_Sleep
system_call_Sleep:
 addiu $2,$0,18
 syscall
 j $31
 .end system_call_Sleep

 .globl system_call_GetTime
 .ent system_call_GetTime
system_call_GetTime:
 addiu $2,$0,19
 syscall
 j $31
 .end system_call_GetTime

 .globl system_call_GetNumInstr
 .ent system_call_GetNumInstr
system_call_GetNumInstr:
 addiu $2,$0,50
 syscall
 j $31
 .end system_call_GetNumInstr

 .globl system_call_PrintIntHex
 .ent system_call_PrintIntHex
system_call_PrintIntHex:
 addiu $2,$0,20
 syscall
 j $31
 .end system_call_PrintIntHex


        .globl __main
        .ent __main
__main:
        j $31
        .end __main
