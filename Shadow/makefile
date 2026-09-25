# --------------------------------------------------------------------
# Shadow Makefile
#
# Windows-native build:
# PowerShell + GNU Make
#
# The source tree is declared in build_config.json (single source of
# truth). generate_build_mks.ps1 turns it into per-directory build.mk
# files plus build/subsystems.mk, which this makefile includes below.
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Shell
# --------------------------------------------------------------------
ifeq ($(OS), Windows_NT)
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command
else
SHELL := /usr/bin/pwsh
.SHELLFLAGS := -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command
endif

# --------------------------------------------------------------------
# Toolchain — x86_64 only, no 32-bit branch anymore
# --------------------------------------------------------------------

TOOLCHAIN := x86_64-elf-

CC       := $(TOOLCHAIN)gcc
C3       := c3c
CXX      := $(TOOLCHAIN)g++
LD       := $(TOOLCHAIN)ld
AR       := $(TOOLCHAIN)ar
NM       := $(TOOLCHAIN)nm
SIZE     := $(TOOLCHAIN)size
OBJDUMP  := $(TOOLCHAIN)objdump
OBJCOPY  := $(TOOLCHAIN)objcopy
READELF  := $(TOOLCHAIN)readelf
RUSTC    ?= rustc
CARGO    ?= cargo
NASM     ?= nasm

# --------------------------------------------------------------------
# Windows filesystem helpers
# --------------------------------------------------------------------

ifeq ($(OS),Windows_NT)

define RM_DIR
    if (Test-Path -LiteralPath '$(1)') { Remove-Item -LiteralPath '$(1)' -Recurse -Force }
endef

define RM_FILE
    if (Test-Path -LiteralPath '$(1)') { Remove-Item -LiteralPath '$(1)' -Force }
endef

else

define RM_DIR
    rm -rf "$(1)"
endef

define RM_FILE
    rm -f "$(1)"
endef

endif

# --------------------------------------------------------------------
# Project layout
# --------------------------------------------------------------------

TARGET        := SHADOW.ELF
BUILD_DIR     := build
LINKER_SCRIPT := link.ld

.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

# NOTE: no SRC_DIRS list here anymore. Which subsystems exist, which
# DRV subdirectories are enabled, and which -D macros each one gets
# all live in build_config.json. Editing that file is the only step;
# the rule at the bottom of this file regenerates everything else.

# --------------------------------------------------------------------
# Compiler flags
# --------------------------------------------------------------------

COMMON_FLAGS := \
    -m64 \
    -mno-red-zone \
    -mgeneral-regs-only \
    -mcmodel=kernel \
    -g \
    -ffreestanding \
    -fno-stack-protector \
    -fno-asynchronous-unwind-tables \
    -fno-unwind-tables \
    -fno-pic \
    -fno-pie \
    -O2 \
    -MMD \
    -MP \
    -IInclude \
    -I../PhononPkg/Boot

# NOTE: no -mcmodel=kernel yet. That model assumes every symbol lives in
# the top 2GiB of address space (0xffffffff80000000+) — correct for the
# OLD Phonon's higher-half layout, wrong for Shadow's current low
# (0x200000, identity-mapped) link address. Bring it back once Shadow
# reintroduces higher-half paging, not before — mixing them risks
# relocation-truncation errors or silently wrong codegen.

# __OWLYNEST__ used to be hardcoded here; it (and the per-subsystem
# -D__TAL__ / -D__INT__ / ... macros) now come from build_config.json
# via BUILD_DEFINES, appended below after the generated include.

CFLAGS := \
    $(COMMON_FLAGS) \
    -Wall \
    -Wextra \
    -Werror \
    -include Include/Internal/Types.H \
    -x c

C3FLAGS := \
    --target elf-x64 \
    --use-stdlib=no \
    --emit-stdlib=no \
    -g0

CXXFLAGS := \
    $(COMMON_FLAGS) \
    -Wall \
    -Wextra \
    -Werror \
    -fno-exceptions \
    -fno-rtti \
    -fno-threadsafe-statics \
    -fno-use-cxa-atexit \
    -std=c++20

RUSTFLAGS := \
    -C panic=abort \
    -C opt-level=2 \
    -C relocation-model=static \
    -C force-frame-pointers=no

NASM_FLAGS := \
    -f ELF64

LDFLAGS := \
    -nostdlib -z noexecstack

LIBGCC := $(shell $(CC) -print-libgcc-file-name)
LIBS := $(LIBGCC)

# --------------------------------------------------------------------
# Generated build files — see build_config.json
#
# build/subsystems.mk is produced by generate_build_mks.ps1 and:
#   - includes each declared subsystem's build.mk (aggregators like
#     DRV/build.mk pull in exactly the subdirs declared in the config)
#   - sets BUILD_DEFINES from the config's "macros"/"global_defines"
#
# -include keeps the first run silent; the rule further down makes
# sure it is (re)generated whenever the config or the generator
# itself changes — including right after `make clean` wipes build/.
# --------------------------------------------------------------------

-include build/subsystems.mk

CFLAGS   += $(BUILD_DEFINES)
CXXFLAGS += $(BUILD_DEFINES)

# Bootstrap: Kernel.C lives at the root for now, before there's a real
# subsystem for it in build_config.json. Delete this line once it
# lands in one of the declared directories.
C_SRCS += Kernel.C

# --------------------------------------------------------------------
# Object lists
# --------------------------------------------------------------------

C_OBJS    := $(patsubst %.C,$(BUILD_DIR)/%.o,$(C_SRCS))
C3_OBJS   := $(patsubst %.C3,$(BUILD_DIR)/%.o,$(C3_SRCS))
CPP_OBJS  := $(patsubst %.CPP,$(BUILD_DIR)/%.o,$(CPP_SRCS))
ASM_OBJS  := $(patsubst %.ASM,$(BUILD_DIR)/%.o,$(ASM_SRCS))
GAS_OBJS  := $(patsubst %.S,$(BUILD_DIR)/%.o,$(GAS_SRCS))
RUST_OBJS := $(patsubst %.rs,$(BUILD_DIR)/%.o,$(RUST_SRCS))

OBJS := \
    $(C_OBJS) \
    $(C3_OBJS) \
    $(CPP_OBJS) \
    $(ASM_OBJS) \
    $(GAS_OBJS) \
    $(RUST_OBJS)

DEPS := $(OBJS:.o=.d)

# --------------------------------------------------------------------
# Targets
# --------------------------------------------------------------------

.PHONY: all clean size sections symbols readelf

all: $(TARGET)

# --------------------------------------------------------------------
# Link
# --------------------------------------------------------------------

$(TARGET): $(OBJS) $(LINKER_SCRIPT)
	@Write-Host "LD  $@"
	@& "$(LD)" $(LDFLAGS) -T $(LINKER_SCRIPT) -o "$@" $(filter-out %.ld,$^) $(LIBS)

# --------------------------------------------------------------------
# C
# --------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.C
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "CC  $<"
	@& "$(CC)" $(CFLAGS) -c "$<" -o "$@"

# --------------------------------------------------------------------
# C3
# --------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.C3
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "C3  $<"
	@& "$(C3)" compile-only "$<" $(C3FLAGS) --obj-out "$(dir $@)" --emit-only "$(basename $(notdir $<))"

# --------------------------------------------------------------------
# C++
# --------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.CPP
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "CXX $<"
	@& "$(CXX)" $(CXXFLAGS) -c "$<" -o "$@"

# --------------------------------------------------------------------
# Assembly (NASM)
# --------------------------------------------------------------------
$(BUILD_DIR)/%.o: %.ASM
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "NASM $<"
	@& "$(NASM)" $(NASM_FLAGS) "$<" -o "$@"

# --------------------------------------------------------------------
# Assembly (GAS)
# --------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.S
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "GAS  $<"
	@& "$(CC)" $(COMMON_FLAGS) -c "$<" -o "$@"

# --------------------------------------------------------------------
# Rust
# --------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.rs
	@New-Item -ItemType Directory -Force -Path "$(dir $@)" | Out-Null
	@Write-Host "RS  $<"
	@& "$(RUSTC)" --crate-type lib --emit=obj $(RUSTFLAGS) -o "$@" "$<"

# --------------------------------------------------------------------
# Generated build files — regeneration rule
# --------------------------------------------------------------------

build/subsystems.mk: build_config.json generate_build_mks.ps1
	@Write-Host "GEN build/subsystems.mk (build_config.json changed)"
	@& "$(CURDIR)/generate_build_mks.ps1"

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------

-include $(DEPS)

# --------------------------------------------------------------------
# Cleaning
# --------------------------------------------------------------------

clean:
	@Write-Host "Cleaning Shadow build..."
	$(call RM_DIR,$(BUILD_DIR))
	$(call RM_FILE,$(TARGET))

# --------------------------------------------------------------------
# Inspection
# --------------------------------------------------------------------

size:
	@& "$(SIZE)" "$(TARGET)"

sections:
	@& "$(OBJDUMP)" -h "$(TARGET)"

symbols:
	@& "$(NM)" -n "$(TARGET)"

readelf:
	@& "$(READELF)" -l "$(TARGET)"
