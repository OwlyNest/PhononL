/*
	* Shadow/Arch/X64/IDT.C - x86_64 IDT and default fault handling
	* Author:   amity
	* Date:     Thu Sep 17 23:39:21 2026
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
#define IDT_GATE_INTERRUPT 0x8E   /* present | dpl0 | 64-bit interrupt gate */

/* --- Includes ---*/
#include <IAL/IAL.H>
#include <Arch/X64/IDT.H>
#include <Lib/PrintK.H>

/* --- Typedefs - Structs - Enums ---*/
typedef struct IDT_ENTRY {
	/* 0 */ UINT16 OffsetLow;
	/* 2 */ UINT16 Selector;
	/* 4 */ UINT8  Ist;
	/* 5 */ UINT8  TypeAttr;
	/* 6 */ UINT16 OffsetMid;
	/* 8 */ UINT32 OffsetHigh;
	/* 12 */ UINT32 Reserved;
	/* 16 */ 
} __attribute__((packed)) _IDT_ENTRY, *_PIDT_ENTRY;
C_ASSERT(sizeof(_IDT_ENTRY) == 16);

typedef struct IDT_POINTER {
	/* 0 */ UINT16 Limit;
	/* 2 */ UINT64 Base;
	/* 10 */ 
} __attribute__((packed)) _IDT_POINTER, *_PIDT_POINTER;
C_ASSERT(sizeof(_IDT_POINTER) == 10);

/* --- Globals ---*/
static _IDT_ENTRY IdtEntries[IDT_ENTRIES];
static _IDT_POINTER IdtPointer;
static INTERRUPT_HANDLER HandlerTable[IDT_ENTRIES];

extern UINT64 IsrStubTable[IDT_ENTRIES];

static PCCHAR FaultNames[32] = {
	"Divide Error",
	"Debug",
	"NMI",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved",
	"x87 FP Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD FP Exception",
	"Virtualization Exception",
	"Control Protection Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"VMM Communication Exception",
	"Security Exception",
};
/* --- Prototypes ---*/
static UINT16 IdtGetCs(VOID);
static VOID   IdtSetGate(IN UINT8 Vector, IN UINT64 Handler, IN UINT16 Selector, IN UINT8 TypeAttr);
static VOID   IdtDefaultFault(IN _PINTERRUPT_FRAME Frame);

/* --- Functions ---*/

/* ==========================================================================
 * Selector: read, not assumed
 *
 * Shadow has no GDT of its own yet; every register dump this whole
 * project has produced shows CS still pointing at firmware's original
 * descriptor, and that remains true here (GDTR was never reloaded).
 * Reading it live rather than hardcoding 0x38 means this keeps working
 * unmodified the day a real GDT replaces firmware's, instead of being a
 * value that was only ever correct by observation.
 * ======================================================================= */

static UINT16 IdtGetCs(VOID) {
	UINT16 Cs;
	__asm__ __volatile__("movw %%cs, %0" : "=r"(Cs));
	return Cs;
}

static VOID IdtSetGate(
	IN UINT8  Vector,
	IN UINT64 Handler,
	IN UINT16 Selector,
	IN UINT8  TypeAttr
) {
	_PIDT_ENTRY Entry = &IdtEntries[Vector];

	Entry->OffsetLow = (UINT16)(Handler & 0xFFFF);
	Entry->Selector  = Selector;
	Entry->Ist       = 0;
	/*
		* No IST stacks yet; see the note
		* this is heading toward once #DF/#NMI
		* need their own known-good stack
	*/

	Entry->TypeAttr   = TypeAttr;
	Entry->OffsetMid  = (UINT16)((Handler >> 16) & 0xFFFF);
	Entry->OffsetHigh = (UINT32)((Handler >> 32) & 0xFFFFFFFF);
	Entry->Reserved   = 0;
}

/* ==========================================================================
 * Default fault handler
 *
 * This is the whole reason an IDT was worth building this early: a real,
 * readable exception with a faulting address, instead of a silent hang
 * or a firmware reset with no diagnostic at all.
 * ======================================================================= */

static VOID IdtDefaultFault(
	IN _PINTERRUPT_FRAME Frame
) {
	UINT64 Cr2 = 0;

	printk("\n[!] %a (vector %lu, error 0x%lx)\n", FaultNames[Frame->Vector], Frame->Vector, Frame->ErrorCode);
	printk("[!] RIP=0x%lx CS=0x%lx RFLAGS=0x%lx\n", Frame->Rip, Frame->Cs, Frame->Rflags);

		if (Frame->Vector == 14) { /* Page Fault */
		__asm__ __volatile__("movq %%cr2, %0" : "=r"(Cr2));

		printk("[!] CR2=0x%lx (%a, %a, %a)\n", Cr2, (Frame->ErrorCode & 1) ? "protection violation" : "not present", (Frame->ErrorCode & 2) ? "write" : "read", (Frame->ErrorCode & 4) ? "user-mode" : "kernel-mode");
	}

	for (;;) {
		__asm__ __volatile__("cli\n\thlt");
	}
}

/* ==========================================================================
 * Dispatch
 *
 * Exceptions (< 32) are never EOI'd; they aren't PIC/APIC-routed, and
 * there is nothing to acknowledge. IRQs (>= 32) ALWAYS get EOI'd, even
 * with no handler registered: skipping it is how a device stops
 * interrupting forever, for reasons that look nothing like a missing EOI
 * by the time they're noticed.
 * ======================================================================= */

VOID IdtDispatch(
	IN _PINTERRUPT_FRAME Frame
) {
	UINT8 Vector = (UINT8)Frame->Vector;

	if (Vector < 32) {
		if (HandlerTable[Vector]) {
			HandlerTable[Vector](Frame);

		} else {
			IdtDefaultFault(Frame);
		}
	return;
	}

	if (HandlerTable[Vector]) {
		HandlerTable[Vector](Frame);
	}

	IALSendEoi(Vector);
}

/* ==========================================================================
 * Initialization
 * ======================================================================= */

VOID IdtInit(VOID) {
	UINT16 Cs = IdtGetCs();
 
	for (UINT32 v = 0; v < IDT_ENTRIES; v++) {
		IdtSetGate((UINT8)v, IsrStubTable[v], Cs, IDT_GATE_INTERRUPT);
		HandlerTable[v] = NULL;
	}
 
	IdtPointer.Limit = sizeof(IdtEntries) - 1;
	IdtPointer.Base  = (UINT64)&IdtEntries[0];
 
	__asm__ __volatile__("lidt %0" :: "m"(IdtPointer));
 
	printk("[Idt] Loaded, %u entries, CS=0x%x\n", IDT_ENTRIES, Cs);
}
 
SHSTATUS IdtRegisterHandler(
	IN UINT8 Vector,
	IN INTERRUPT_HANDLER Handler
) {
	if (Vector >= IDT_ENTRIES) {
		return (SHSTATUS)-1;
	}
 
	HandlerTable[Vector] = Handler;
	return STATUS_SUCCESS;
}