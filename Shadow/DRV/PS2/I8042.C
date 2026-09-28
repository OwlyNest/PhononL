/*
	* Shadow/DRV/PS2/I8042.C - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 28 02:12:30 2026
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

/*
	* The I8042 is a second, tiny computer running in parallel with the main one.
	* The CPU executes this function; meanwhile the U8042 is sitting between the keyboard and the bus,
	* clocking in bits. These two machines never run in lockstep; they synchronize through one shared flag.
	* The whole driver is bookkeeping around that fact.
*/

/* --- Macros ---*/

/* --- Includes ---*/
#include <DRV/PS2/I8042.H>
#include <DRV/PS2/PS2.H>
#include <Int/IO.H>
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
/*
	* Cached result of the port-2 probe, run once by init. FALSE until
	* proven otherwise: a controller that ignores 0xA8 is treated as
	* single-port, and the mouse stays deferred. Port 1 needs no cache:
	* every 8042 has one.
*/
static BOOLEAN I8042Port2Present = FALSE;

/* --- Prototypes ---*/

/* --- Functions ---*/
/*
	* Here's the journey of one keypress, end to end:
	*
	* 1:
	* User presses a key. The keyboard's internal microcontroller emits a scancode byte one bit at a time,
	* a UART frame of 11 bits: start bit, 8 data bits, parity bit, stop bit, clocked at 10–16.7 kHz.
	* One byte takes roughly a millisecond to cross the wire.
	*
	* 2:
	* The 8042 shifts those bits in. When the frame completes, it places the byte in its output buffer,
	* which is exactly one byte deep, a mailbox, not a queue, and raises its output-buffer-full flag (status bit 0).
	* If IRQ1 is enabled in the config byte, it also asserts the interrupt line.
	*
	* 3:
	* The drivers side of the contract is two registers: peek at 0x64 to read the flag, pop 0x60 to take the byte.
	* Crucially, these have different side-effect profiles: 
	*     Reading 0x64 is a pure look; it disturbs nothing. 
	*     Reading 0x60 is a destructive pop, it empties the mailbox and drops the flag (and deasserts IRQ1).
	* 
	* Three more hardware facts that shape the code:
	*
	* Errors are per-byte, not per-transaction.
	*     Status bits 6 (timeout) and 7 (parity) describe the byte currently sitting in the buffer.
	*     A set flag means "the byte in the mailbox is garbage", a bad frame arrived.
	*     Waiting longer cannot fix this; the corruption already happened. 
	*     So: pop it, report FALSE.
	*
	* The garbage must be popped.
	*     IRQ1 is level-triggered off the output-buffer-full condition.
	*     If you observe a bad byte and walk away without reading 0x60,
	*     the flag stays set and the interrupt re-fires forever. 
	*     Consuming the byte is not courtesy, it's how you clear the condition.
	* The reason Timeout == 0 exists:
	*     the IRQ handler calls this from interrupt context where spinning is legal but
	*     unbounded waiting is not; a lost byte must not wedge the machine.
	*     The main loop calls it with a generous bound and treats FALSE as "nothing yet."
*/
BOOLEAN I8042ReadData(
	IN UINT32 Timeout, /* */
	OUT UINT8 *Byte    /* The single byte that is read from the Data port*/
) {
	UINT32 Spin;
	UINT8  Status;

	/*
		* Poll once, then spin up to Timeout more times
		*Timeout == 0
		*    loop body runs exactly once → one poll, never waits.
		* Timeout == N
		*     one poll plus N spins.
		* The +1 buys "first attempt is free," which is what makes the same code correct both
		* in the IRQ handler (0) and in bring-up code (large N).
	*/
	for (Spin = Timeout + 1; Spin != 0; --Spin) {
		Status = InByte(PS2_STATUS_REGISTER);
		if (Status & PS2_SR_OUTPUT_BUFFER_STATUS) {
			break; /* Mail arrived */
		}
	}

	if (Spin == 0) {
		return FALSE; /* Mailbox stayed empty */
	}

	/*
		* Only check the error inside the break branch, not per-iteration.
		* The parity/timeout flags are only meaningful when OBF is set; they qualify the byte in the buffer.
		* Checking them on an empty buffer would read phantom errors.
	*/
	if (Status & (PS2_SR_PARITY_ERROR | PS2_SR_TIME_OUT_ERROR)) {
		(VOID)InByte(PS2_DATA_PORT); /* Pop garbage, clears OBF + IRQ1 */
		return FALSE;
	}

	*Byte = InByte(PS2_DATA_PORT); /* The pop */
	return TRUE;
}

/*
	* Mirror of I8042ReadData, roles inverted: we produce, the 8042 consumes.
	* IBF (status bit 1) is the 8042's "mail still uncollected" flag, set by
	* the chip when it latches a byte, cleared by the chip when it has taken
	* the byte into its own scratchpad. The CPU never clears it; polling IBF
	* is polling the other machine's progress. Writing while it is set
	* overwrites the uncollected byte.
	*
	* TRUE means the 8042 accepted custody, not that any device received
	* the byte, confirmed receipt is the 0xFA ACK arriving later through
	* the output buffer (see I8042DeviceCommand). No error flags exist for
	* the write itself; failures surface later on response bytes.
*/

BOOLEAN I8042WriteData(
	IN UINT8 Byte /* For PORT1 device, or controller command argument */
) {
	UINT32 Spin;

	/*
		* Same shape as the read's loop: poll once, then spin.
		* The check is inverted: wait for IBF clear, not set. 
		* A fixed internal timeout, because this only runs from process context.
	*/
	for (Spin = I8042_SPIN_TIMEOUT + 1; Spin != 0; --Spin) {
		if (!(InByte(PS2_STATUS_REGISTER) & PS2_SR_INPUT_BUFFER_STATUS)) {
			OutByte(PS2_DATA_PORT, Byte);
			return TRUE;
		}
	}

	return FALSE;
}

/*
	* Literally I8042WriteData aimed at the other register: same input
	* buffer, same IBF discipline. Only the meaning changes, and the
	* meaning was decided by the address, not the byte, the 8042 sets
	* status bit 3 itself based on which port the byte arrived on.
	*
	* Commands are requests, not payloads: TRUE means the 8042 accepted
	* the command byte. Commands with a response (0x20-0x3F, 0xA9, 0xAA,
	* 0xAB, 0xAC, 0xC0, 0xD0) deliver it later through the SAME output
	* buffer as keyboard bytes and ACKs, one door for everything
	* returning. The caller owns the sequence knowledge of what to expect
	* and follows with I8042ReadData.
*/
BOOLEAN I8042Command(
	IN UINT8 Command /* Command byte: the PS2_CMD_* vocabulary */
) {
	UINT32 Spin;

	/* Same loop as I8042WriteData: wait for IBF clear, then out. */
	for (Spin = I8042_SPIN_TIMEOUT + 1; Spin != 0; --Spin) {
		if (!(InByte(PS2_STATUS_REGISTER) & PS2_SR_INPUT_BUFFER_STATUS)) {
			OutByte(PS2_COMMAND_REGISTER, Command);
			return TRUE;
		}
	}

	return FALSE;
}

/*
	* Two-byte commands (0x60-0x7F, 0xD1, 0xD4): command to 0x64,
	* argument to 0x60. The IBF check inside I8042WriteData is exactly
	* the table's "make sure bit 1 is clear before the next byte".
	* Writing both back-to-back could clobber the command before the
	* 8042 collected it. The composition isn't convenience, it's the
	* required inter-write wait.
*/
BOOLEAN I8042CommandArg(
	IN UINT8 Command, /* Command to write */
	IN UINT8 Arg /* Second byte: goes to the data port */
) {
	if (!I8042Command(Command)) {
		return FALSE;
	}

	return I8042WriteData(Arg);
}

/*
	* Why a prefix: the second PS/2 port is a hardware afterthought, the
	* ISA I/O map had no spare data-port address, so port 2 is reached
	* through the SAME data port as port 1, steered by command 0xD4:
	* "route the NEXT data byte to port 2". That makes 0xD4 a modal
	* command, it re-routes one future write, which is why this function
	* is shaped prefix-then-payload.
	*
	* The payload goes through I8042WriteData rather than a raw OutByte
	* because the IBF re-check between the two writes is part of the
	* contract, the table's "make sure the controller is ready for it".
	*
	* Retry rule: the pair succeeds or fails as a unit. If the prefix
	* went out but the payload timed out, retry both, a lone data byte
	* is relying on a modal state you can no longer observe.
*/
BOOLEAN I8042WriteDevice(
	IN UINT8 Port, /* 1 = data port, 2 = 0xD4 prefix + data port */
	IN UINT8 Byte  /* Payload for whichever device lives on Port */
) {
	if (Port < 1 || Port > 2) {
		return FALSE; /* Contract violation, costs zero bus cycles */
	}

	if (Port == 2) {
		if (!I8042Command(PS2_CMD_SEND_PORT2)) {
			return FALSE;
		}
	}

	return I8042WriteData(Byte);
}

/*
	* The first conversation, not action: custody transfer
	* (I8042WriteDevice) plus end-to-end confirmation from the device
	* itself. The device checks the parity bit on the wire; 0xFE is a
	* retry request, not an error, and obligates re-sending the SAME
	* byte, custody transfer and all. 0xFC/0xFD are fatal. Anything
	* else means the conversation desynced: consumed, reported FALSE,
	* the caller decides what to do. Bounded: at most MAX_RESENDS+1
	* sends, a device that never answers cleanly must not wedge us.
	*
	* Note the read timeout: I8042_RESP_TIMEOUT, not I8042_SPIN_TIMEOUT.
	* Two timescales live in this chip: the i8042 answers in
	* microseconds, devices in ~milliseconds of wire time.
*/
BOOLEAN I8042DeviceCommand(
	IN UINT8 Port, 
	IN UINT8 Byte /* Command byte for the device on Port */
) {
	UINT8  Response;
	UINT32 Retries;

	for (Retries = I8042_MAX_RESENDS + 1; Retries != 0; --Retries) {

		if (!I8042WriteDevice(Port, Byte)) {
			return FALSE; /* Bus side failed: no point waiting */
		}

		if (!I8042ReadData(I8042_RESP_TIMEOUT, &Response)) {
			return FALSE; /* Device never answered */
		}

		if (Response == PS2_DEV_ACK) {
			return TRUE;
		}

		if (Response == PS2_DEV_RESEND) {
			continue; /* Retry request: same byte, from the top */
		}

		return FALSE; /* 0xFC/0xFD error or desync: consumed*/
	}

	return FALSE; /* RESENDs exhausted */
}

/*
	* "Usable" means the controller HAS the port, not that the port is
	* enabled -- liveness is EnablePort's business. Cheap: the probe ran
	* once at init, hardware capability does not change at runtime.
	* Note the probe itself never calls this function; init pokes 0xA8
	* through raw I8042Command before any capability is known, so there
	* is no bootstrap circularity in the PortUsable check below.
*/
BOOLEAN I8042PortUsable(
	IN UINT8 Port /* 1 exists everywhere; 2 is the cached probe result */
) {
	if (Port == 1) {
		return TRUE;
	}

	if (Port != 2) {
		return FALSE; /* Contract violation, not "unusable" */
	}

	return I8042Port2Present;
}

/*
	* Make port N live, in the only order that cannot lose a byte:
	*
	*   1. Drain stale output. Whatever is in the buffer belongs to the
	*      previous conversation.
	*   2. Config byte read-modify-write: set the IRQ bit (doorbell),
	*      CLEAR the clock-disable bit, active-low trap, 1 means
	*      DISABLED. Never write a literal: the byte holds the other
	*      port's bits, the system flag, and translation.
	*   3. Controller enable (0xAE / 0xA8) LAST. Mute switches come off
	*      only when everything is armed. Call LAST in bring-up, after
	*      the IRQ handler is registered: this is the moment bytes and
	*      interrupts start flowing.
*/
_PS2_Status I8042EnablePort(
	IN UINT8 Port /* 1 or 2; 2 is refused while the probe says absent */
) {
	UINT8 Config;
	UINT8 Scratch;

	if (Port < 1 || Port > 2 || !I8042PortUsable(Port)) {
		return PS2_ERR_INVALID_PARAMETER;
	}

	/* Poll-once reads: safe anywhere, error bytes consumed for free */
	while (I8042ReadData(0, &Scratch)) {
	}

	if (!I8042Command(PS2_CMD_READ_CFG_BYTE)) {
		return PS2_ERR_DEVICE;
	}

	if (!I8042ReadData(I8042_SPIN_TIMEOUT, &Config)) {
		return PS2_ERR_DEVICE;
	}

	if (Port == 1) {
		Config |= PS2_CCB_PORT1_INT;    /* Doorbell on */
		Config &= ~PS2_CCB_PORT1_CLOCK; /* Unmute    */
	} else {
		Config |= PS2_CCB_PORT2_INT;
		Config &= ~PS2_CCB_PORT2_CLOCK;
	}

	if (!I8042CommandArg(PS2_CMD_WRITE_CFG_BYTE, Config)) {
		return PS2_ERR_DEVICE;
	}

	if (Port == 1) {
		return I8042Command(PS2_CMD_ENABLE_PORT1) ? PS2_SUCCESS : PS2_ERR_DEVICE;
	}

	return I8042Command(PS2_CMD_ENABLE_PORT2) ? PS2_SUCCESS : PS2_ERR_DEVICE;
}

/*
	* Exact inverse, and the order inverts with it: mute FIRST, disarm
	* last. 0xAD / 0xA7 before the config write, so a byte arriving in
	* the window waits in the buffer with its handler still registered
	* and the doorbell still wired, nothing lost. Then clear the IRQ
	* bit and SET the clock-disable bit. After this returns, the port is
	* as quiet as the controller can make it. For failed bring-up.
*/
_PS2_Status I8042DisablePort(
	IN UINT8 Port
) {
	UINT8 Config;

	if (Port < 1 || Port > 2 || !I8042PortUsable(Port)) {
		return PS2_ERR_INVALID_PARAMETER;
	}

	if (Port == 1) {
		if (!I8042Command(PS2_CMD_DISABLE_PORT1)) {
			return PS2_ERR_DEVICE;
		}
	} else {
		if (!I8042Command(PS2_CMD_DISABLE_PORT2)) {
			return PS2_ERR_DEVICE;
		}
	}

	if (!I8042Command(PS2_CMD_READ_CFG_BYTE)) {
		return PS2_ERR_DEVICE;
	}

	if (!I8042ReadData(I8042_SPIN_TIMEOUT, &Config)) {
		return PS2_ERR_DEVICE;
	}

	if (Port == 1) {
		Config &= ~PS2_CCB_PORT1_INT;  /* Doorbell off */
		Config |= PS2_CCB_PORT1_CLOCK; /* Mute       */
	} else {
		Config &= ~PS2_CCB_PORT2_INT;
		Config |= PS2_CCB_PORT2_CLOCK;
	}

	return I8042CommandArg(PS2_CMD_WRITE_CFG_BYTE, Config) ? PS2_SUCCESS : PS2_ERR_DEVICE;
}

/*
	* Read-modify-write's read, and the cheapest existence proof: a
	* live 8042 always answers 0x20. Absence is only ever visible as
	* silence -- writes to a missing controller succeed vacuously, so
	* the reads are where reality is checked.
*/
static BOOLEAN I8042ReadConfig(
	OUT UINT8 *Config
) {
	if (!I8042Command(PS2_CMD_READ_CFG_BYTE)) {
		return FALSE;
	}

	return I8042ReadData(I8042_SPIN_TIMEOUT, Config);
}

/*
	* Controller bring-up. One ordered conversation, made possible by
	* the first two steps: the room is made quiet BEFORE any question
	* is asked, so sequence knowledge owns every byte that follows.
	*
	*   1. Port gates off (0xAD, 0xA7): devices muted instantly. 0xA7
	*      is a no-op on single-port controllers -- harmless.
	*   2. IRQ bits off in the config byte: doorbells disarmed.
	*      Whatever firmware left pending can no longer assert IRQ1.
	*   3. Drain: anything already in the buffer belongs to the
	*      firmware era (including a keyboard still finishing its
	*      power-on BAT -- its 0xAA is consumed here without ceremony;
	*      0x55 and 0xAA are distinct values, so expectations stay
	*      unambiguous).
	*   4. Self-test 0xAA, expect 0x55. On some hardware the test
	*      RESETS the controller and the config byte reverts -- which
	*      is why the full config program happens after, never before.
	*   5. Final config: doorbells off, clocks muted, translation ON.
	*      See PS2.H: translation is controller state that defines
	*      KBD.C's scancode dialect (Set 1). One correct value
	*      kernel-wide, so the controller owns it. (Port 2 never
	*      translates; MOUSE.C must parse raw Set 2.)
	*   6. Port-2 probe: 0xA8, then watch whether config bit 5 obeyed
	*      (capability by side effect). 0xA7, then confirm the bit set
	*      again; a controller that contradicts itself is not trusted.
	*   7. Interface tests: 0xAB must answer 0x00 or the keyboard is
	*      electrically dead and the whole init fails. 0xA9 if port 2
	*      exists: a failure there only re-defers the mouse.
	*
	* Ports are left DISABLED. Liveness is per-device bring-up: KBD.C
	* registers its handler, then I8042EnablePort(1) un-mutes the
	* world in the safe order. The controller never touches Int.
*/
static SHSTATUS I8042Init(VOID) {
	UINT8 Config;
	UINT8 Scratch;

	I8042Command(PS2_CMD_DISABLE_PORT1);
	I8042Command(PS2_CMD_DISABLE_PORT2);

	if (!I8042ReadConfig(&Config)) {
		return PS2_ERR_DEVICE; /* Nobody home: no 8042 */
	}

	Config &= ~(PS2_CCB_PORT1_INT | PS2_CCB_PORT2_INT); /* Disarm */

	if (!I8042CommandArg(PS2_CMD_WRITE_CFG_BYTE, Config)) {
		return PS2_ERR_DEVICE;
	}

	while (I8042ReadData(0, &Scratch)) { /* Drain the firmware era */
	}

	if (!I8042Command(PS2_CMD_TEST_CONTROLLER)) {
		return PS2_ERR_DEVICE;
	}

	if (!I8042ReadData(I8042_RESP_TIMEOUT, &Scratch)) {
		return PS2_ERR_DEVICE;
	}

	if (Scratch != 0x55) {
		return PS2_ERR_DEVICE; /* 0xFC: self-test failed */
	}

	if (!I8042ReadConfig(&Config)) { /* May have reverted: re-read */
		return PS2_ERR_DEVICE;
	}

	Config &= ~(PS2_CCB_PORT1_INT | PS2_CCB_PORT2_INT); /* Doorbells */
	Config |= PS2_CCB_PORT1_CLOCK | PS2_CCB_PORT2_CLOCK; /* Muted   */
	Config |= PS2_CCB_PORT1_TRANS; /* KBD.C speaks Set 1, see PS2.H */

	if (!I8042CommandArg(PS2_CMD_WRITE_CFG_BYTE, Config)) {
		return PS2_ERR_DEVICE;
	}

	/* Probe port 2 by ordering it to change, and watching */
	I8042Command(PS2_CMD_ENABLE_PORT2);

	if (!I8042ReadConfig(&Config)) {
		return PS2_ERR_DEVICE;
	}

	if (!(Config & PS2_CCB_PORT2_CLOCK)) { /* Bit obeyed: it listened */
		I8042Port2Present = TRUE;
		I8042Command(PS2_CMD_DISABLE_PORT2);

		if (!I8042ReadConfig(&Config) || !(Config & PS2_CCB_PORT2_CLOCK)) {
			I8042Port2Present = FALSE; /* Contradicted itself */
		}
	}

	if (!I8042Command(PS2_CMD_TEST_PORT1)) {
		return PS2_ERR_DEVICE;
	}

	if (!I8042ReadData(I8042_RESP_TIMEOUT, &Scratch) || Scratch != 0x00) {
		return PS2_ERR_DEVICE; /* Clock/data line stuck */
	}

	if (I8042Port2Present) {
		if (I8042Command(PS2_CMD_TEST_PORT2) &&
			I8042ReadData(I8042_RESP_TIMEOUT, &Scratch) &&
			Scratch == 0x00) {
			/* Electrically fine; still deferred until MOUSE.C */
		} else {
			I8042Port2Present = FALSE; /* Broken: keep it deferred */
		}
	}

	return STATUS_SUCCESS;
}

XSCOPENODE(I8042, I8042Init);