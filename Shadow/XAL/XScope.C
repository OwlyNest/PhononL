/*
	* Shadow/XAL/XScope.C - dependency-ordered subsystem bring-up
	* Author:   amity
	* Date:     Sun Sep 20 13:37:04 2026
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
#define XSCOPE_MAX_NODES 64   /* subsystem count, not a hot-path limit. Bump freely if it's ever actually hit */

/* --- Includes ---*/
#include <Lib/Lib.H>
#include <XAL/XScope.H>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/*
	* Collected by Link.ld from every XSCOPE_NODE(...) in the kernel.
	* KEEP() is required in the linker script for this to survive --
	* nothing ever references an individual node by symbol name, only by
	* walking this range, so a garbage-collecting link (--gc-sections)
	* would discard every one of them as "unused" without it.
*/

extern CONST _XScopeNode *CONST __XScopeNodesStart[];
extern CONST _XScopeNode *CONST __XScopeNodesEnd[];

/* --- Prototypes ---*/
static BOOLEAN XScopeNameIsDone(
	IN CONST _XScopeNode *CONST *Nodes,
	IN CONST BOOLEAN *Done,
	IN SIZE_T Count,
	IN PCCHAR Name
);

/* --- Functions ---*/

static BOOLEAN XScopeNameIsDone(
	IN CONST _XScopeNode *CONST *Nodes,
	IN CONST BOOLEAN *Done,
	IN SIZE_T Count,
	IN PCCHAR Name
) {
	for (SIZE_T i = 0; i < Count; i++) {
		if (Done[i] && StrCmp(Nodes[i]->Name, Name) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

VOID XScopeRun(VOID) {
	CONST _XScopeNode *CONST *Nodes = __XScopeNodesStart;
	SIZE_T Count = (SIZE_T)(__XScopeNodesEnd - __XScopeNodesStart);

	if (Count == 0 || Count > XSCOPE_MAX_NODES) {
		printk("[XScope] node count %lu out of range, halting\r\n",
			(UINT64)Count);
		for (;;) { __asm__ __volatile__("cli\n\thlt"); }
	}

	/*
	 * Padding between input sections would show up as NULL "entries";
	 * a bogus gather would show up as NULL Name/Init. Either way the
	 * failure mode is this message, not a #GP deep in bring-up.
	 */
	for (SIZE_T i = 0; i < Count; i++) {
		if (Nodes[i] == NULL ||
			Nodes[i]->Name == NULL ||
			Nodes[i]->Init == NULL) {
			printk("[XScope] malformed node table at entry %lu, halting\r\n",
				(UINT64)i);
			for (;;) { __asm__ __volatile__("cli\n\thlt"); }
		}
	}

	BOOLEAN Done[XSCOPE_MAX_NODES];
	MemSet(Done, 0, sizeof(Done));

	SIZE_T Remaining = Count;

	while (Remaining > 0) {
		SIZE_T Best = Count;   /* "none chosen yet" sentinel */

		for (SIZE_T i = 0; i < Count; i++) {
			if (Done[i]) {
				continue;
			}

			CONST _XScopeNode *Node = Nodes[i];
			BOOLEAN Ready = TRUE;

			for (SIZE_T d = 0; d < Node->DependCount; d++) {
				if (!XScopeNameIsDone(Nodes, Done, Count, Node->Depends[d])) {
					Ready = FALSE;
					break;
				}
			}

			if (!Ready) {
				continue;
			}

			/* Among everything ready RIGHT NOW, lowest Priority wins */
			if (Best == Count || Node->Priority < Nodes[Best]->Priority) {
				Best = i;
			}
		}

		if (Best == Count) {
			printk("[XScope] Stuck: unresolved dependency or cycle among:\r\n");
			for (SIZE_T i = 0; i < Count; i++) {
				if (!Done[i]) {
					printk("  - %s\r\n", Nodes[i]->Name);
				}
			}
			for (;;) { __asm__ __volatile__("cli\n\thlt"); }
		}

		CONST _XScopeNode *Node = Nodes[Best];
		printk("[XScope] %s...\r\n", Node->Name);

		if (Node->Init() != STATUS_SUCCESS) {
			printk("[XScope] %s failed to initialize, halting\r\n", Node->Name);
			for (;;) { __asm__ __volatile__("cli\n\thlt"); }
		}

		Done[Best] = TRUE;
		Remaining--;
	}

	printk("[XScope] All %lu subsystems initialized\r\n", (UINT64)Count);
}