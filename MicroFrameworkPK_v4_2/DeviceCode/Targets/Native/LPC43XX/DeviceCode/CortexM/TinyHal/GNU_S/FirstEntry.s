@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@  Licensed under the Apache License, Version 2.0 (the "License")@
@  you may not use this file except in compliance with the License.
@  You may obtain a copy of the License at http:@www.apache.org/licenses/LICENSE-2.0
@
@  Copyright (c) Microsoft Corporation. All rights reserved.
@  Implementation for LPC43XX: Micromint USA <support@micromint.com>
@
@  CORTEX-M Standard Entry Code 
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.syntax unified

    .global  EntryPoint

    .global  PreStackInit_Exit_Pointer
    .global  ARM_Vectors

    .global StackBottom
    .global StackTop
    .global HeapBegin
    .global HeapEnd
    .global CustomHeapBegin
    .global CustomHeapEnd

    .extern  PreStackInit


    .ifdef HAL_REDUCESIZE
        .extern BootEntryLoader
    .else
        .extern BootEntry
    .endif
    
    .extern  BootstrapCode

    @*************************************************************************

    .section SectionForStackBottom, "a", %progbits
StackBottom:
    @ DCD 0
    .word 0

    .section SectionForStackTop, "a", %progbits
StackTop:
    @DCD 0
    .word 0
    
    .section SectionForHeapBegin, "a", %progbits
HeapBegin:
    @DCD 0
    .word 0

    .section SectionForHeapEnd, "a", %progbits
HeapEnd:
    @DCD 0
    .word 0

    .section SectionForCustomHeapBegin, "a", %progbits
ARM_Vectors:
    .space   84*4   @ exception vector table (max 84 entries)
CustomHeapBegin:
    @DCD 0
    .word 0

    .section SectionForCustomHeapEnd, "a", %progbits
CustomHeapEnd:
    @DCD 0
    .word 0

    .section i.EntryPoint, "ax", %progbits

@ Signature word used to identify application blocks
Sign_Value = 0x2000E00C

__Vectors:
    .word     StackTop      @ 0 Top of Stack
    .word     Start         @ 1 Reset Handler
    .word     Fault_Handler @ 2 NMI Handler
    .word     Fault_Handler @ 3 Hard Fault Handler
    .word     Fault_Handler @ 4 MPU Fault Handler
    .word     Fault_Handler @ 5 Bus Fault Handler
    .word     Fault_Handler @ 6 Usage Fault Handler
    .word     Sign_Value    @ 7 Reserved
    .word     Fault_Handler @ 8 Reserved
    .word     Fault_Handler @ 9 Reserved
    .word     Fault_Handler @ 10 Reserved
    .word     Fault_Handler @ 11 SVCall Handler
    .word     Fault_Handler @ 12 Debug Monitor Handler
    .word     Fault_Handler @ 13 Reserved
    .word     Fault_Handler @ 14 PendSV Handler
    .word     Fault_Handler @ 15 SysTick Handler

Fault_Handler:
    .thumb_func
    b       Fault_Handler

EntryPoint:
    .thumb_func
    bl      BootstrapCode
    .ifdef HAL_REDUCESIZE
        b   BootEntryLoader
    .else
        b   BootEntry
    .endif

    .balign   4

@ User Initial Stack & Heap

PreStackEntry:
    b       PreStackInit

PreStackInit_Exit_Pointer:
    ldr     sp,StackTop_Ptr
    bx      lr

StackTop_Ptr:
    .word     StackTop

    .end
