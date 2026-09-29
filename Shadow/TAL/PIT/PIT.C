/*
	* Shadow/TAL/PIT.C - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 23 00:44:40 2026
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
#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41 // not for kernel use
#define PIT_CHANNEL_2 0x42 // BEEP!
#define PIT_COMMAND   0x43

#define PIT_BASE 1193182

/*
	* From:
	* Title : "Peripheral Components"
	* Publisher : Intel Corporation
	* ISBN : 1-55512-127-6
	* 
	* =============================================================================
	*  The PIT Ports 
	* ===============
	* 
	* The PIT chip is hooked up to the Intel CPU through the following ports:
	* 
	*                   Port   Description                    
	*                 -----------------------------------------
	*                   40h    Channel 0 counter (read/write) 
	*                   41h    Channel 1 counter (read/write) 
	*                   42h    Channel 2 counter (read/write) 
	*                   43h    Control Word (write only)      
	*                 -----------------------------------------*
	* 
	* =============================================================================
	*  The Control Word 
	* ==================
	* 
	*               ---------------------------------
	*               | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	*               ---------------------------------
	*                 \___/   \___/   \_______/   \-- BCD 0 - Binary 16 bit
	*                   |       |         |               1 - BCD 4 decades
	* ------------------------  |         |
	* | Select Counter       |  |         \---------- Mode Number 0 - 5
	* | 0 - Select Counter 0 |  |
	* | 1 - Select Counter 1 |  |         ------------------------------
	* | 2 - Select Counter 2 |  |         | Read/Load                  |
	* ------------------------  |         | 0 - Counter Latching       |
	*                           \---------| 1 - Read/Load LSB only     |
	*                                     | 2 - Read/Load MSB only     |
	*                                     | 3 - Read/Load LSB then MSB |
	*                                     ------------------------------
	* 
	* =============================================================================
	*  The PIT Modes 
	* ===============
	* 
	* The PIT is capable of operating in 6 different modes:
	* 
	* MODE 0 - Interrupt on Terminal Count
	* ------------------------------------
	* 
	* When this mode is set the output will be low. Loading the count register
	* with a value will cause the output to remain low and the counter will start
	* counting down. When the counter reaches 0 the output will go high and remain
	* high until the counter is reprogrammed. The counter will continue to count
	* down after terminal count is reached. Writing a value to the count register
	* during counting will stop the counter, writing a second byte starts the
	* new count.
	* 
	* MODE 1 - Programmable One-Shot
	* ------------------------------
	* 
	* The output will go low once the counter has been loaded, and will go high
	* once terminal count has been reached. Once terminal count has been reached
	* it can be triggered again.
	* 
	* MODE 2 - Rate Generator
	* -----------------------
	* 
	* A standard divide-by-N counter. The output will be low for one period of the
	* input clock then it will remain high for the time in the counter. This cycle
	* will keep repeating.
	* 
	* MODE 3 - Square Wave Rate Generator
	* -----------------------------------
	* 
	* Similar to mode 2, except the ouput will remain high until one half of the
	* count has been completed and then low for the other half.
	* 
	* MODE 4 - Software Triggered Strobe
	* ----------------------------------
	* 
	* After the mode is set the output will be high. Once the count is loaded it
	* will start counting, and will go low once terminal count is reached.
	* 
	* MODE 5 - Hardware Triggered Strobe
	* ----------------------------------
	* 
	* Hardware triggered strobe. Similar to mode 5, but it waits for a hardware
	* trigger signal before starting to count.
	* 
	* Modes 1 and 5 require the PIT gate pin to go high in order to start
	* counting. I'm not sure if this has been implemented in the PC.
*/

/* --- Includes ---*/
#include <TAL/TAL.H>
#include <IAL/IAL.H>

#include <Int/IO.H>
#include <Int/Int.H>
#include <Lib/Lib.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static UINT64 Ticks;

/* --- Prototypes ---*/
static INT 	  TalPitInit(IN _PTAL_BACKEND Self);
static UINT32 TalPitSetFrequency(IN _PTAL_BACKEND Self, IN UINT32 DesiredHz);
static VOID   TalPitStop(IN _PTAL_BACKEND Self);
static UINT8  TalPitGetVector(IN _PTAL_BACKEND Self);
static UINT64 TalPitGetTicks(IN _PTAL_BACKEND Self);

static _TAL_BACKEND TAL_PIT = {
	.Name = "Programmable Interval Timer",
	.Init = TalPitInit,
	.SetFrequency = TalPitSetFrequency,
	.Stop = TalPitStop,
	.GetVector = TalPitGetVector,
	.GetTicks = TalPitGetTicks,
};

/* --- Functions ---*/
VOID TalPitHandler(
	IN _PINTERRUPT_FRAME Frame
) {
	(VOID)Frame;
	Ticks++;
}

_PTAL_BACKEND TalPitBackend(VOID) {
	return &TAL_PIT;
}

static INT TalPitInit(
	IN _PTAL_BACKEND Self
) {
	(VOID)Self;
	Ticks = 0;
	IdtRegisterHandler(Self->GetVector(Self), TalPitHandler);
	IALEnableIrq(0);
	return 0;
}	

static UINT32 TalPitSetFrequency(
	IN _PTAL_BACKEND Self,
	IN UINT32 DesiredHz
) {
	(VOID)Self;
	if (DesiredHz == 0) {
		/* wat? */
		return 0;
	}

	UINT32 Divisor = PIT_BASE / DesiredHz;

	if (Divisor > 0xFFFF) {
		Divisor = 0;      /* hardware special case: 0 means 65536, the
		                   * lowest representable rate, ~18.2 Hz */
	} else if (Divisor == 0) {
		Divisor = 1;      /* highest representable rate, ~1.19 MHz */
	}
	/*
		* Control Word 0x34 = 0b00110100
		* Looking at the diagram above:
		* 00-11-010-0
		* Select counter 0
		* Read/Load LSB then MSB
		* Mode 2: Rate Generator
		* Binary 16 bit
	*/
	OutByte(PIT_COMMAND, 0x34);
	OutByte(PIT_CHANNEL_0, Divisor & 0xFF);
	OutByte(PIT_CHANNEL_0, Divisor >> 8);

	UINT32 ActualDivisor = (Divisor == 0) ? 65536 : Divisor;
	return PIT_BASE / ActualDivisor;
}

static VOID   TalPitStop(
	IN _PTAL_BACKEND Self
) {
	(VOID)Self;
	
	IALDisableIrq(0);
}


static UINT8  TalPitGetVector(
	IN _PTAL_BACKEND Self
) {
	(VOID)Self;
	/*
		* PIT is always IRQ0 relative to whichever controller IAL has
	 	* active, PIC today, self-vectored APIC timer later won't even
	 	* go through this path.
	*/
	return IALGetVectorBase() + 0;
}

static UINT64 TalPitGetTicks(
	IN _PTAL_BACKEND Self
 ) {
	(VOID)Self;
	return Ticks;
}