# Shadow/Arch/X64/ISR.S - [Enter description]
# Author:   amity
# Date:     Thu Sep 17 23:45:07 2026
# Copyright © 2026 OwlyNest

# ==============================
# Alignment instructions:

# Instructions               begin at col 5
# First operand              begins at col 10
# Second operand             begins at col 20 (if possible)

# Header comments            begin at col 5
# In-line comments           begin at col 40
# Labels                     begin at col 1, end with ':'
# Section dividers           use 30 '=' characters
# (Sub)Section titles        use 3 '=' characters before and after

.altmacro

.macro ISR_NOERR num
.globl Isr\num
Isr\num:
    push $0
    push $\num
    jmp IsrCommon
.endm

.macro ISR_ERR num
.globl Isr\num
Isr\num:
    push $\num
    jmp IsrCommon
.endm

.macro ISR_TABLE_ENTRY num
    .quad Isr\num
.endm

.text

.set i, 0
.rept 32
    .if i==8 || i==10 || i==11 || i==12 || i==13 || i==14 || i==17
        ISR_ERR %i
    .else
        ISR_NOERR %i
    .endif
    .set i, i+1
.endr

.set i, 32
.rept 16
    ISR_NOERR %i
    .set i, i+1
.endr

/* ==========================================================================
 * Common trampoline
 *
 * Push order is the reverse of INTERRUPT_FRAME's field order on purpose:
 * the last thing pushed ends up at the lowest address, i.e. offset 0,
 * i.e. Frame->R15. RSP handed to IdtDispatch IS the frame, no copy.
 *
 * No stack realignment before the call: every C file in this kernel is
 * built with -mgeneral-regs-only, so the compiler can never emit an
 * SSE instruction that would care about 16-byte alignment. That's the
 * one thing that makes skipping it here actually safe rather than just
 * convenient. If that flag is ever dropped, this needs revisiting.
 * ======================================================================= */

IsrCommon:
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %rbp
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15
 
	movq %rsp, %rdi
	call IdtDispatch
 
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rbp
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
 
	add $16, %rsp        /* discard vector + error code */
	iretq

.section .data
.align 8
.globl IsrStubTable
IsrStubTable:
.set i, 0
.rept 48
    ISR_TABLE_ENTRY %i
    .set i, i+1
.endr