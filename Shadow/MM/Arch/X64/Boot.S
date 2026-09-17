# Shadow/MM/Arch/X64/Boot.S - low entry stub: initial page tables
# Author:   amity
# Date:     Wed Sep 16 14:23:53 2026
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


# PhononBoot leaves us in long mode with firmware's identity map and
# PhononBootInfo* in RDI. Everything here runs at its physical address
# (0x200000-ish) and MUST NOT touch any high-linked symbol: those pages
# do not exist yet.
#
# Builds a deliberately crude map -- three static tables, 1 GiB pages,
# no allocator involved, because none exists yet:
#
#   PML4[0]   -> 512 GiB identity      (temporary: the memory map, the
#                                       framebuffer and this very code
#                                       all still live down there)
#   PML4[256] -> the same 512 GiB      (MM_DIRECT_MAP_BASE)
#   PML4[511] -> phys 0, 1 GiB         (MM_KERNEL_BASE)
#
# MmInitPaging() replaces all of it with properly sized, properly
# permissioned tables as soon as the PMM is up.
#
# Requires 1 GiB page support (CPUID 0x80000001 EDX bit 26). Every
# x86_64 part Shadow targets has it, and old Phonon already depended on
# it, but that is the assumption to revisit first if this triple-faults
# on some machine.
#

.section .boot.data, "aw", @progbits
.align 4096
BootPml4:
	.space 4096
BootDirectPdpt:
	.space 4096
BootKernelPdpt:
	.space 4096
 
.section .boot.text, "ax", @progbits
.code64
.globl _start
.type _start, @function
 
_start:
	cli
	cld	
 
	# PhononBootInfo* survives in R15 across the table build.
	movq %rdi, %r15
 
	# --- Zero all three tables (they are contiguous) ---
	leaq BootPml4(%rip), %rdi
	xorq %rax, %rax
	movq $(3 * 512), %rcx
	rep stosq
 
	#--- DirectPdpt: 512 entries x 1 GiB = 512 GiB, identity ---
	leaq BootDirectPdpt(%rip), %rdi
	xorq %rax, %rax                     # running physical base
	movq $512, %rcx
.Lfill_direct:
	movq %rax, %rdx
	orq  $0x83, %rdx                    # Present | Writable | PageSize
	movq %rdx, (%rdi)
	addq $8, %rdi
	addq $0x40000000, %rax
	decq %rcx
	jnz .Lfill_direct
 
	# --- KernelPdpt[510] -> physical 0, 1 GiB ---
	# PDPT index 510 under PML4[511] is exactly 0xFFFFFFFF80000000.
	# Separate table from the direct map precisely because index 510
	# would otherwise collide with the identity fill above.
	
	leaq BootKernelPdpt(%rip), %rdi
	movq $0x83, %rax
	movq %rax, (510 * 8)(%rdi)
 
	# --- PML4 ---
	leaq BootPml4(%rip), %rdi
 
	leaq BootDirectPdpt(%rip), %rax
	orq  $0x03, %rax                    # Present | Writable
	movq %rax, (0 * 8)(%rdi)            # identity, dropped later
	movq %rax, (256 * 8)(%rdi)          # MM_DIRECT_MAP_BASE
 
	leaq BootKernelPdpt(%rip), %rax
	orq  $0x03, %rax
	movq %rax, (511 * 8)(%rdi)          # MM_KERNEL_BASE
 
	# --- EFER.NXE ---
    # Firmware already sets this (confirmed in the register dump), but
	# every NX bit we are about to write depends on it, so do not
	# inherit that as an assumption.
	
	movl $0xC0000080, %ecx
	rdmsr
	orl  $0x800, %eax
	wrmsr
 
	/* --- Go live --- */
	leaq BootPml4(%rip), %rax
	movq %rax, %cr3
 
	# Identity is still mapped, so the next instruction is fine either
	# way; the jump is what actually moves execution to the high half.
	
	movq %r15, %rdi
	movabsq $BootHighEntry, %rax
	jmpq *%rax
 
.size _start, . - _start
 
# ==========================================================================
# From here on we are high-linked and high-executing.
# =======================================================================
.section .text
.code64
.type BootHighEntry, @function
 
BootHighEntry:
	# Firmware's stack is still mapped through the identity range, but it
	# is firmware's, and it disappears the moment MmInitPaging drops
	# that range. Move to our own before calling anything.
	
	movabsq $KernelStackTop, %rsp
	xorq %rbp, %rbp
 
	call KernelMain
 
	# KernelMain must not return. If it does, stop cleanly rather than
	# executing whatever follows in .text.
	#

	cli
.Lhalt:
	hlt
	jmp .Lhalt
 
.size BootHighEntry, . - BootHighEntry
 
.section .bss, "aw", @nobits
.align 16
KernelStack:
	.space 65536
KernelStackTop:
 