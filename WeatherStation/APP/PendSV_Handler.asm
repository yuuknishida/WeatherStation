.thumb
    .ref currentTask
    .ref OS_Scheduler

    .thumbfunc PendSV_Handler
    .global PendSV_Handler


PendSV_Handler:
    MRS     r0, PSP
    STMDB   r0!, {r4-r11}
    LDR     r1, currentTask_addr
    LDR     r2, [r1]
    STR     r0, [r2]

	PUSH	{lr}
    BL      OS_Scheduler
    POP 	{LR}

    LDR     r1, currentTask_addr
    LDR     r2, [r1]
    LDR     r0, [r2]
    LDMIA   r0!, {r4-r11}
    MSR     PSP, r0
    BX      lr

currentTask_addr .word currentTask

; -------------------------------------------------------
; OS_Launch - start the first task
;
; IMPORTANT: EXC_RETURN (BX 0xFFFFFFFD) only works when the CPU
; is in Handler mode (inside an actual exception handler).
; OS_Launch is called from main() = Thread mode, so a naked
; BX with an EXC_RETURN value from here triggers a HardFault,
; not an exception return.  This was the root cause of the crash
; (R3=0xFFFFFFFD, garbage LR, PC landing in the vector table).
;
; Fix: issue SVC #0 to enter the SVC handler in Handler mode,
; then do the real first-task launch from there.
; -------------------------------------------------------
    .thumbfunc OS_Launch
    .global OS_Launch
OS_Launch:
    SVC     #0          ; software interrupt -> CPU enters SVC_Handler in Handler mode
    BX      lr          ; never reached; keeps assembler happy

; -------------------------------------------------------
; SVC_Handler
;
; Called by OS_Launch via SVC #0.  We are now in Handler mode,
; so EXC_RETURN is valid.
;
; 1. Load currentTask->stkPtr
; 2. Pop R4-R11 (software frame)
; 3. Set PSP to base of hardware frame
; 4. Switch Thread mode to use PSP (CONTROL.SPSEL = 1)
; 5. EXC_RETURN — CPU unstacks R0-R3/R12/LR/PC/xPSR from PSP
;    and resumes the first task.
; -------------------------------------------------------
    .thumbfunc SVC_Handler
    .global SVC_Handler
SVC_Handler:
    LDR     r1, currentTask_addr2
    LDR     r2, [r1]            ; r2 = currentTask (pointer value)
    LDR     r0, [r2]            ; r0 = currentTask->stkPtr (offset 0 in task_t)

    LDMIA   r0!, {r4-r11}       ; pop software frame; r0 now at hardware frame base

    MSR     PSP, r0             ; PSP points at saved R0 (base of hardware frame)
    MOV     r0, #2              ; CONTROL = 2: Thread mode uses PSP, privileged
    MSR     CONTROL, r0
    ISB                         ; barrier required after CONTROL write

    MVN     lr, #2              ; lr = 0xFFFFFFFD = EXC_RETURN: Thread/PSP/no-FPU
    BX      lr                  ; valid EXC_RETURN from Handler mode -> starts task

currentTask_addr2 .word currentTask
