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
#include <Lib/PrintK.H>
#include <Lib/String.H>
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

extern CONST _XScopeNode __XScopeNodesStart[];
extern CONST _XScopeNode __XScopeNodesEnd[];

/* --- Prototypes ---*/
static BOOLEAN XScopeNameIsDone(IN CONST _XScopeNode *Nodes, IN CONST BOOLEAN *Done, IN SIZE_T Count, IN PCCHAR Name);

/* --- Functions ---*/

static BOOLEAN XScopeNameIsDone(
		IN CONST _XScopeNode *Nodes,
	IN CONST BOOLEAN *Done,
	IN SIZE_T Count,
	IN PCCHAR Name
) {
	for (SIZE_T i = 0; i < Count; i++) {
		if (Done[i] && StrCmp(Nodes[i].Name, Name) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

VOID XScopeRun(VOID) {
	CONST _XScopeNode *Nodes = __XScopeNodesStart;
	SIZE_T Count = (SIZE_T)(__XScopeNodesEnd - __XScopeNodesStart);

	BOOLEAN Done[XSCOPE_MAX_NODES];
	MemSet(Done, 0, sizeof(Done));

	SIZE_T Remaining = Count;

	while (Remaining > 0) {
		SIZE_T Progress = 0;

		for (SIZE_T i = 0; i < Count; i++) {
			if (Done[i]) {
				continue;
			}

			CONST _XScopeNode *Node = &Nodes[i];
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

			printk("[XScope] %a...\n", Node->Name);
 
			if (Node->Init() != STATUS_SUCCESS) {
				printk("[XScope] %a failed to initialize, halting\n", Node->Name);
				for (;;) { __asm__ __volatile__("cli\n\thlt"); }
			}
 
			Done[i] = TRUE;
			Remaining--;
			Progress++;
		}

		if (Progress == 0) {
			/*
				* Every full pass over the remaining nodes changed nothing:
				* either a Depends name doesn't match any registered node
				* (a typo, or the node it names was never linked in), or two
				* or more nodes depend on each other. Report all of them.
				* Pointing at which one is "the" problem would be a guess.
			*/

			printk("[XScope] Stuck: unresolved dependency or cycle among:\n");
			for (SIZE_T i = 0; i < Count; i++) {
				if (!Done[i]) {
					printk("  - %a\n", Nodes[i].Name);
				}
			}
			for (;;) { __asm__ __volatile__("cli\n\thlt"); }
		}
	}

	printk("[XScope] All %lu subsystems initialized\n", (UINT64)Count);
}