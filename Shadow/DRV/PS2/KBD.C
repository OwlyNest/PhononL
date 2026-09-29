/*
	* Shadow/DRV/PS2/KBD.C - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 28 02:12:00 2026
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

/* --- Includes ---*/
#include <DRV/PS2/I8042.H>
#include <IAL/IAL.H>
#include <DRV/PS2/PS2.H>
#include <DRV/PS2/KBD.H>
#include <XAL/XScope.H>
#include <Int/Int.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
/*
	* Single producer (the IRQ handler), single consumer (the main
	* loop), one CPU: two word-sized indices are the entire
	* synchronization. No locks, no atomics, no volatile -- the
	* producer can only preempt the consumer between whole calls,
	* and each call touches each index exactly once.
*/
static _PS2_KEY KbdRing[KBD_RING_SIZE];
static UINT32   KbdRingHead;       /* Producer (IRQ) writes here   */
static UINT32   KbdRingTail;       /* Consumer (PS2KeyboardRead)   */
static UINT32   KbdRingDropped;    /* Overflow diagnostic          */

static BOOLEAN KbdExtendedPending; /* 0xE0 seen: next byte is extended */
static BOOLEAN KbdSysRqPending;
static UINT8   KbdPauseRemaining = PS2_SET1_PAUSE_LEN;

/* --- Prototypes ---*/

/* --- Functions ---*/

/*
	* Non-blocking: FALSE means "no key event waiting", not an error.
	* Safe to call from the main loop while the IRQ handler is
	* producing.
*/
BOOLEAN PS2KeyboardRead(
	OUT _PPS2_KEY Key /* Receives the oldest pending event */
) {
	if (KbdRingHead == KbdRingTail) {
		return FALSE;
	}

	*Key = KbdRing[KbdRingTail];
	KbdRingTail = (KbdRingTail + 1) % KBD_RING_SIZE;
	return TRUE;
}

/*
	* Overflow policy: drop the INCOMING event, never evict an
	* old one. Evicting would silently reorder the stream; dropping
	* keeps every byte that survived in order. Either policy can
	* theoretically strand a press without its release (a stuck-key
	* illusion downstream) -- 32 slots makes this a starved-scheduler
	* diagnostic path, not a design problem. The drop counter exists
	* so bring-up can prove it never fires.
*/
static VOID KbdRingPush(
	IN _PPS2_KEY Key
) {
	UINT32 Next = (KbdRingHead + 1) % KBD_RING_SIZE;

	if (Next == KbdRingTail) {
		KbdRingDropped++;
		return;
	}

	KbdRing[KbdRingHead] = *Key;
	KbdRingHead = Next;
}

/*
	* IRQ1 handler: the interrupt is a notification, not a delivery.
	* Drain the mailbox with poll-once reads (Timeout == 0: never waits,
	* safe by construction), push events, EOI, leave. The machine is
	* interrupt-dark while this runs; every cycle here is stolen from
	* every other device. Parse state (prefix flags) is written ONLY
	* here -- single writer, no sharing, no locks, same discipline as
	* the ring indices.
	*
	* Known approximations (documented, not solved): Print Screen
	* (E0 2A E0 37) arrives as two extended events; Pause (E1 + 5
	* bytes) arrives as six plain events. Both are problems for
	* whatever layer turns scancodes into keys, PS2_KEY is
	* deliberately raw, and no keymap table belongs in the driver.
*/
static VOID KbdIrqHandler(
	IN _PINTERRUPT_FRAME Frame
) {
	UINT8 Byte;
	_PS2_KEY Event;

	(VOID)Frame;

	/*
		* Bounded by hardware capacity, not a counter: each pass pops
		* exactly one byte from a one-byte-deep buffer
	*/
	while (I8042ReadData(0, &Byte)) {
		/*
			* Pause: swallow the rest of the 6 byte sequence.
			* No release exists for Pause in hardware.
		*/
		if (KbdPauseRemaining != 0) {
			--KbdPauseRemaining;
			continue;
		}

		if (Byte == PS2_SET1_PAUSE) {
			Event.Scancode = PS2_SET1_PAUSE; /* synthetic ID */
			Event.Pressed  = TRUE;
			Event.Extended = FALSE;
			KbdPauseRemaining = PS2_SET1_PAUSE_LEN - 1;
			KbdRingPush(&Event);
			continue;
		}

		if (Byte == PS2_SET1_EXTENDED) {
			KbdExtendedPending = TRUE;
		}

		/*
			* Print Screen, first half (E0 2A / E0 AA): arm the pair,
			* emit nothing yet. E0 2A never occurs for any other key.
		*/
		if (KbdExtendedPending && !KbdSysRqPending && (Byte & ~PS2_SET1_BREAK) == PS2_SET1_SYSRQ_ID) {
			KbdSysRqPending    = TRUE;
			KbdExtendedPending = FALSE;
			continue;
		}

		/* Print Screen, second half: direction from bit 7 */
		if (KbdSysRqPending && KbdExtendedPending) {
			Event.Scancode = PS2_SET1_SYSRQ_ID;
			Event.Pressed  = !(Byte & PS2_SET1_BREAK);
			Event.Extended = TRUE;
			KbdSysRqPending    = FALSE;
			KbdExtendedPending = FALSE;
			KbdRingPush(&Event);
			continue;
		}

		/* Plain (possibly extended) byte: the common path */
		Event.Scancode = Byte & ~PS2_SET1_BREAK;
		Event.Pressed  = !(Byte & PS2_SET1_BREAK);
		Event.Extended = KbdExtendedPending;
		KbdExtendedPending = FALSE;
		KbdRingPush(&Event);
	}


	/*
		* No EOI needed, IDT dispatcher already does that
	*/
}


/*
	* Degradation contract (the XScope lesson): init only fails the
	* machine when the kernel cannot proceed. A missing or broken
	* keyboard is not that. Every failure path here cleans up and
	* returns SUCCESS: the keyboard layer loads as a no-op and
	* PS2KeyboardRead returns FALSE forever. Nobody outside can tell
	* "no keyboard" from "no keys pressed".
*/
SHSTATUS KbdInit(VOID) {
	UINT8 Config;
	UINT8 Id0;
	UINT8 Id1;

	/*
		* Handler exists but cannot fire: the 8042 doorbell is disarmed
		* by I8042 init, and the PIC stays masked until someone unmasks
		* IRQ1. Vector comes from the backend, never a literal.
	*/
	IdtRegisterHandler(IALGetVectorBase() + 1, KbdIrqHandler);

	if (!I8042PortUsable(1)) {
		return STATUS_SUCCESS; /* No controller, or no port 1 */
	}

	/*
		* Unmute, minus the doorbell: the quiet half of I8042EnablePort
		* done by hand, because the conversation below must happen with
		* interrupts impossible. EnablePort is idempotent and becomes
		* the finale: it arms what is already set, and drains.
	*/
	while (I8042ReadData(0, &Id0)) { /* Drain strays */
	}

	if (!I8042Command(PS2_CMD_READ_CFG_BYTE)) {
		goto Degrade;
	}

	if (!I8042ReadData(I8042_SPIN_TIMEOUT, &Config)) {
		goto Degrade;
	}

	Config &= ~PS2_CCB_PORT1_CLOCK; /* Unmute */

	if (!I8042CommandArg(PS2_CMD_WRITE_CFG_BYTE, Config)) {
		goto Degrade;
	}

	if (!I8042Command(PS2_CMD_ENABLE_PORT1)) {
		goto Degrade;
	}

	if (I8042EnablePort(1) != PS2_SUCCESS) {
		goto Degrade;
	}

		/* The conversation. Doorbell off: replies wait in the buffer for
		our polls, the handler cannot steal them. */
	if (!I8042DeviceCommand(1, PS2_KBD_CMD_DISABLE_SCAN)) {
		goto Degrade; /* Key held during bring-up would spam */
	}

	if (!I8042DeviceCommand(1, PS2_DEV_CMD_RESET)) {
		goto Degrade;
	}

	/*
		* BAT: the one truly slow step, hundreds of ms. 0xAA = passed.
		* (DeviceCommand already consumed the 0xFA ACK.)
	*/
	if (!I8042ReadData(I8042_RESP_TIMEOUT, &Id0) || Id0 != 0xAA) {
		goto Degrade;
	}

		if (!I8042DeviceCommand(1, PS2_DEV_CMD_IDENTIFY)) {
		goto Degrade;
	}

	/*
		* Translation is on (I8042 init set it): expect the translated
		* MF2 IDs. Anything else on port 1 is not a keyboard.
	*/
	if (!I8042ReadData(I8042_RESP_TIMEOUT, &Id0) ||
		Id0 != PS2_ID_KB_PREFIX) {
		goto Degrade;
	}

	if (!I8042ReadData(I8042_RESP_TIMEOUT, &Id1) ||
		(Id1 != PS2_ID_KB_MF2 && Id1 != PS2_ID_KB_MF2_ALT)) {
		goto Degrade;
	}

	/* Reset defaults leave scanning OFF until told otherwise. */
	if (!I8042DeviceCommand(1, PS2_KBD_CMD_ENABLE_SCAN)) {
		goto Degrade;
	}

	/*
		* Finale: idempotent over an already-live port.
		* After this call the doorbell is armed and bytes ring IRQ1.
	*/
	if (I8042EnablePort(1) != STATUS_SUCCESS) {
		goto Degrade;
	}


	IALEnableIrq(1);
	return STATUS_SUCCESS;
	/*
		* Don't disable PS/2 Port 1 on success.
	*/

Degrade:
	I8042DisablePort(1);
	return STATUS_SUCCESS;
} 

XSCOPENODE(PS2_KBD, KbdInit, "X64_IDT", "I8042", "IAL");