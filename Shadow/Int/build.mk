# Auto-generated aggregator for Int — do not edit.
# Add/remove subsystems in build_config.json, not here.

C_SRCS += Int/GDT.C
C_SRCS += Int/IDT.C
ASM_SRCS += Int/GDTFLUSH.ASM
GAS_SRCS += Int/ISR.S

-include Int/CPUID/build.mk
