;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Licensed under the Apache License, Version 2.0 (the "License");
;  you may not use this file except in compliance with the License.
;  You may obtain a copy of the License at http:;www.apache.org/licenses/LICENSE-2.0
;
;  Copyright (c) Microsoft Corporation. All rights reserved.
;  Implementation for LPC43XX: Micromint USA <support@micromint.com>
;
;  CORTEX-M Standard Entry Code 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    EXPORT  EntryPoint

    EXPORT  ARM_Vectors
    EXPORT  HeapBegin
    EXPORT  HeapEnd
    EXPORT  SlaveBegin
    EXPORT  SlaveEnd
    EXPORT  StackBottom
    EXPORT  StackTop

    IMPORT  PreStackInit
    EXPORT  PreStackInit_Exit_Pointer

    IF HAL_REDUCESIZE = "1"
        IMPORT  BootEntryLoader
    ELSE
        IMPORT  BootEntry
    ENDIF
    IMPORT  BootstrapCode

    PRESERVE8

;*******************************************************************************
; Allocate RAM for exception vectors, stack, heap.
; Set base addresses in scatter file
Max_Vectors     EQU     (16 + 68)          ; Max vectors (Core + MCU Peripherals)
Vect_Size       EQU     (Max_Vectors * 4)  ; RAM for exception vectors
Heap_Size       EQU     ((128 * 1024) - Vect_Size) ; RAM for heap
Stack_Size      EQU     (16 * 1024)        ; RAM for stack
Slave_Size      EQU     (16 * 1024)        ; RAM for slave cores

    AREA SectionForVectors,        DATA
VectorsBegin
ARM_Vectors       SPACE   Vect_Size
VectorsEnd

    AREA SectionForHeap,              DATA
__heap_base
HeapBegin         DCD     0
HeapEnd           EQU     (HeapBegin + Heap_Size - 8)
__heap_limit      EQU     HeapEnd

    AREA SectionForStack,             DATA
StackBottom       DCD     0
StackTop          EQU     (StackBottom + Stack_Size - 8)
__initial_sp      EQU     StackTop

    AREA SectionForSlave,             DATA
SlaveBegin        DCD     0
SlaveEnd          EQU     (SlaveBegin + Slave_Size)

;*******************************************************************************
; Minimal vector table in flash. Relocated to RAM after boot
    AREA ||i.EntryPoint||, CODE, READONLY
    EXPORT  __Vectors

; Signature word used to identify application blocks
Sign_Value      EQU     0x2000E00C

    ENTRY

__Vectors
    DCD     StackTop      ;  0 Top of Stack
    DCD     EntryPoint    ;  1 Reset Handler
    DCD     Fault_Handler ;  2 NMI Handler
    DCD     Fault_Handler ;  3 Hard Fault Handler
    DCD     Fault_Handler ;  4 MPU Fault Handler
    DCD     Fault_Handler ;  5 Bus Fault Handler
    DCD     Fault_Handler ;  6 Usage Fault Handler
    DCD     Sign_Value    ;  7 Reserved
    DCD     Fault_Handler ;  8 Reserved
    DCD     Fault_Handler ;  9 Reserved
    DCD     Fault_Handler ; 10 Reserved
    DCD     Fault_Handler ; 11 SVCall Handler
    DCD     Fault_Handler ; 12 Debug Monitor Handler
    DCD     Fault_Handler ; 13 Reserved
    DCD     Fault_Handler ; 14 PendSV Handler
    DCD     Fault_Handler ; 15 SysTick Handler

Fault_Handler
    b       .

EntryPoint
    LDR		R0, =BootstrapCode
    BLX		R0
    IF HAL_REDUCESIZE = "1"
       LDR     R0, =BootEntryLoader
    ELSE
       LDR     R0, =BootEntry
    ENDIF
    BX      R0

    ALIGN

; User Initial Stack & Heap

    IF      :DEF:__MICROLIB

    EXPORT  __initial_sp
    EXPORT  __heap_base
    EXPORT  __heap_limit

    ELSE

    IMPORT  __use_two_region_memory
    EXPORT  __user_initial_stackheap

__user_initial_stackheap

PreStackEntry
    b       PreStackInit
PreStackInit_Exit_Pointer

    LDR     R0, = HeapBegin
    LDR     R1, = StackTop
    LDR     R2, = HeapEnd
    LDR     R3, = StackBottom
    BX      LR

    ALIGN

    ENDIF

    END
