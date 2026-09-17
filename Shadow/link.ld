OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;
KERNEL_LMA = 0x200000;

PHDRS
{
    boot PT_LOAD FLAGS(5);   /* R+X */
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = KERNEL_LMA;

    .boot ALIGN(4K) : {
        __BootStart = .;
        KEEP(*(.boot.text))
        KEEP(*(.boot.data))
        KEEP(*(.boot*))
        __BootEnd = .;
    } : boot

    ASSERT(__BootEnd > __BootStart, "boot section empty – Boot.S sections not matched")

    . = ALIGN(4K);
    . += KERNEL_VMA;

    __KernelVirtStart = .;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA) {
        __TextStart = .;
        *(.text*)
        . = ALIGN(4K);
        __TextEnd = .;
    } : text

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA) {
        __RodataStart = .;
        *(.rodata*)
        . = ALIGN(4K);
        __RodataEnd = .;
    } : text

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA) {
        __DataStart = .;
        *(.data*)
        . = ALIGN(4K);
        __DataEnd = .;
    } : data

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA) {
        __BssStart = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4K);
        __BssEnd = .;
    } : data

    __KernelVirtEnd = .;
    __KernelVmaMarker = KERNEL_VMA;

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}