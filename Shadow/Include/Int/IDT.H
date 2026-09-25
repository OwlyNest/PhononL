/*
	* Shadow/Include/Int/IDT.H - [Enter description]
	* Author:   amity
	* Date:     Thu Sep 17 23:39:13 2026
	* Copyright © 2026 OwlyNest
*/

/* --- Styling Instructions ---
	* Encoding:                      UTF-8, Unix line endings
	* Text font:                     Monospace
	* Line width:                    Max 80 characters
	* Indentation:                   Use 4 spaces
	* Brace style:                   Same line as control statement
	* Inline comments:               Column 40, wherever possible, else, whole multiple of 20
	* Section headers:               Use 3 '-' characters before and after
	* Pointer notation:              Next to variable name, not type
	* Binary operations:             Space around operator
	* Empty parameter list:          Use (void) instead of ()
	* Statements and declarations:   Max one per line
*/

/* --- Macros ---*/
#ifndef __INT_IDT_H__
#define __INT_IDT_H__

#define IDT_ENTRIES 48   /* 32 exceptions + 16 legacy IRQs for now, extend when MSI wants arbitrary vectors */

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/*
	* What a handler actually receives. Field order matches Isr.S's push
	* order exactly, Frame is RSP itself, cast, not a copy. Reordering
	* one without the other silently reads garbage.
*/

typedef struct INTERRUPT_FRAME {
	UINT64 R15;
	UINT64 R14;
	UINT64 R13;
	UINT64 R12;
	UINT64 R11;
	UINT64 R10;
	UINT64 R9;
	UINT64 R8;
	UINT64 Rbp;
	UINT64 Rdi;
	UINT64 Rsi;
	UINT64 Rdx;
	UINT64 Rcx;
	UINT64 Rbx;
	UINT64 Rax;
 
	UINT64 Vector;
	UINT64 ErrorCode;    /* 0 for vectors that don't push a real one */
 
	/* Pushed by the CPU itself, not by us */
	UINT64 Rip;
	UINT64 Cs;
	UINT64 Rflags;
	UINT64 Rsp;
	UINT64 Ss;
} _INTERRUPT_FRAME, *_PINTERRUPT_FRAME;

typedef VOID (*INTERRUPT_HANDLER)(IN _PINTERRUPT_FRAME Frame);

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif /* __INT_IDT_H__ */