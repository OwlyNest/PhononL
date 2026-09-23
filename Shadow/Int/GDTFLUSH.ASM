; Shadow/Int/GDTFLUSH.ASM - [Enter description]
; Author:   amity
; Date:     Thu Sep 24 00:08:09 2026
; Copyright © 2026 OwlyNest
; ------------------------------
; Alignment instructions:

; Instructions               begin at col 5
; First operand              begins at col 10
; Second operand             begins at col 15 (if possible)

; Header comments            begin at col 5
; In-line comments           begin at col 40
; Labels                     begin at col 1, end with ':'
; Section dividers           use 30 '-' characters
; (Sub)Section titles        use 3 '-' characters before and after

; ------------------------------
; --- Macros ---

; ------------------------------
; --- Globals ---

; ------------------------------
; --- Externs ---

; ------------------------------
; --- Code ---
global GdtFlush
GdtFlush:
    lgdt [rdi]              ; gp: limit(2) + base(8), packed
    mov ax, 0x10            ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08               ; kernel code selector
    lea rax, [rel .Flush]
    push rax
    retfq                   ; far ret: pops RIP, then CS
.Flush:
    ret
; ------------------------------
; --- Helpers ---

; ------------------------------
; --- Tests ---

; ------------------------------
; --- Buffers ---

; ------------------------------
; --- Messages ---
