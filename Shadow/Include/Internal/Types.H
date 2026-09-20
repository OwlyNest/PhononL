/*
 * include/internal/phonon_types.h - Fundamental kernel types
 * Author:   amity
 * Date:     Tue Aug  4 15:28:52 2026
 * Copyright © 2026 OwlyNest
 */

/* --- Styling Instructions ---
 * Encoding:                      UTF-8, Unix line endings
 * Text font:                     Monospace
 * Line width:                    Max 80 characters
 * Indentation:                   Use 4 spaces
 * Brace style:                   Same line as control statement
 * Inline comments:               Column 40, wherever possible, else, whole
 *                                 multiple of 20
 * Section headers:               Use 3 '-' characters before and after
 * Pointer notation:              Next to variable name, not type
 * Binary operations:             Space around operator
 * Empty parameter list:          Use (void) instead of ()
 * Statements and declarations:   Max one per line
 */

/* --- Macros ---*/
#ifndef __INTERNAL_TYPES_H__
#define __INTERNAL_TYPES_H__

/* --- Architecture Detection --- */
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
#define ARCH_X86_64 1
#define ARCH_BITS 64
#define ARCH_X86 1
#else
#error "Unsupported architecture"
#endif

#define CONST const

/* --- Includes ---*/
#include <stddef.h>
#include <stdint.h>

/* --- Fundamental Integer Types ---*/

/*
 * Fixed-width C types provide the underlying representation.
 */

typedef char CHAR;
typedef uint8_t UCHAR;

typedef int16_t SHORT;
typedef uint16_t USHORT;

typedef int32_t LONG;
typedef uint32_t ULONG;

typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;

typedef int8_t CCHAR;

typedef int8_t  INT8;
typedef uint8_t UINT8;

typedef int16_t  INT16;
typedef uint16_t UINT16;

typedef int32_t  INT32;
typedef uint32_t UINT32;

typedef int64_t  INT64;
typedef uint64_t UINT64;

/* --- Standard Integer Aliases ---*/

typedef int INT;
typedef unsigned UINT;

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

typedef uint64_t ULONG64;
typedef int64_t LONG64;

typedef uint64_t DWORD64;

/* --- Boolean ---*/

typedef uint8_t BOOLEAN;

#define FALSE ((BOOLEAN)0)
#define TRUE ((BOOLEAN)1)

/* --- Pointer-Sized Integer Types ---*/

typedef uintptr_t ULONG_PTR;
typedef intptr_t LONG_PTR;

typedef uintptr_t UINT_PTR;
typedef intptr_t INT_PTR;

typedef ULONG_PTR SIZE_T;
typedef LONG_PTR SSIZE_T;

typedef SIZE_T *PSIZE_T;

/* --- Character Pointers ---*/

typedef CHAR *PCHAR;
typedef UCHAR *PUCHAR;

typedef const CHAR *PCCHAR;
typedef const UCHAR *PCUCHAR;

typedef CHAR *PSTR;
typedef const CHAR *PCSTR;

typedef UCHAR *PBYTE;
typedef const UCHAR *PCBYTE;

/* --- Generic Pointers ---*/

typedef void VOID;

typedef VOID *PVOID;
typedef const VOID *PCVOID;

typedef PVOID *PPVOID;

/* --- Handle Types ---*/

/*
 * Handles are intentionally opaque. Kernel objects must not be exposed
 * through their implementation type to callers.
 */

typedef struct _HANDLE {
  VOID *Object;
} HANDLE;

typedef HANDLE *PHANDLE;

/* --- Status Types ---*/

typedef LONG SHSTATUS;
typedef SHSTATUS *PSHSTATUS;

#define STATUS_SUCCESS ((SHSTATUS)0)

/* --- Access Types ---*/

typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

/* --- Processor / Interrupt Types ---*/

typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

typedef CHAR KPROCESSOR_MODE;

#define KernelMode ((KPROCESSOR_MODE)0)
#define UserMode ((KPROCESSOR_MODE)1)

/* --- Physical Addresses ---*/

/*
 * Keep physical addresses distinct from ordinary integer values.
 */

typedef struct _PHYSICAL_ADDRESS {
  LONGLONG QuadPart;
} PHYSICAL_ADDRESS;

typedef PHYSICAL_ADDRESS *PPHYSICAL_ADDRESS;

#if ARCH_X86_64
typedef ULONGLONG PHYS_ADDR_T, *PPHYS_ADDR_T;
#else
typedef ULONG PHYS_ADDR_T, *PPHYS_ADDR_T;
#endif

/*
 * Page-frame number. Always pointer-width so
 * `(frame << PAGE_SHIFT)` cannot truncate above 4 GiB.
 */
typedef ULONG_PTR PFN_T;
typedef PFN_T *PPFN_T;

/*
 * Virtual address as an integer. Distinct from PHYS_ADDR_T:
 * the two are not interchangeable once paging is on.
 */
typedef ULONG_PTR VIRT_ADDR_T;
typedef VIRT_ADDR_T *PVIRT_ADDR_T;

#define PHYS_ADDR_INVALID ((PHYS_ADDR_T)0)

/* --- Large Integer Types ---*/

typedef union _LARGE_INTEGER {
  struct {
    ULONG LowPart;
    LONG HighPart;
  };

  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  struct {
    ULONG LowPart;
    ULONG HighPart;
  };

  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

/* --- Time Types ---*/

typedef LONGLONG _TIME;
typedef _TIME *P_TIME;

/* --- GUID ---*/

typedef struct _GUID {
  ULONG Data1;
  USHORT Data2;
  USHORT Data3;
  UCHAR Data4[8];
} GUID;

typedef GUID *PGUID;
typedef const GUID *PCGUID;

/* --- Unicode / String Types ---*/

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  USHORT *Buffer;
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _ANSI_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PCHAR Buffer;
} ANSI_STRING;

typedef ANSI_STRING *PANSI_STRING;
typedef const ANSI_STRING *PCANSI_STRING;

/* --- List Entries ---*/

typedef struct _LIST_ENTRY {
  struct _LIST_ENTRY *Flink;
  struct _LIST_ENTRY *Blink;
} LIST_ENTRY;

typedef LIST_ENTRY *PLIST_ENTRY;

/* --- Callback Types ---*/

typedef VOID (*PVOID_FUNCTION)(VOID);

typedef VOID (*PKSTART_ROUTINE)(PVOID Context);

/* --- Function Annotations ---*/

/*
 * These annotations are documentation for humans and static analysis.
 * They intentionally expand to nothing for the compiler.
 */

#define IN
#define OUT
#define OPTIONAL

#define INOUT
#define IN_OPT
#define OUT_OPT

/* --- Compiler Hints ---*/

#if defined(__GNUC__) || defined(__clang__)
#define FORCEINLINE static inline __attribute__((always_inline))
#define NORETURN __attribute__((noreturn))
#define ALIGN(N) __attribute__((aligned(N)))
#else
#define FORCEINLINE static inline
#define NORETURN
#define ALIGN(N)
#endif

/* --- Utility Macros ---*/

#define UNREFERENCED_PARAMETER(Parameter) ((void)(Parameter))

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

// From Google I guess, someone copied it on stackoverflow
#define ARRAY_SIZE(a)                                                          \
  ((sizeof(a) / sizeof(*(a))) / (size_t)(!(sizeof(a) % sizeof(*(a)))))

#define FIELD_OFFSET(Type, Field) offsetof(Type, Field)

/* --- Compile-Time Assertions ---*/

#if defined(__cplusplus)

#define C_ASSERT(Expression) static_assert((Expression), #Expression)

#else

#define C_ASSERT(Expression) _Static_assert((Expression), #Expression)

#endif

/* --- Fundamental Type Guarantees ---*/

C_ASSERT(sizeof(UCHAR) == 1);
C_ASSERT(sizeof(USHORT) == 2);
C_ASSERT(sizeof(ULONG) == 4);
C_ASSERT(sizeof(ULONGLONG) == 8);

C_ASSERT(sizeof(BYTE) == 1);
C_ASSERT(sizeof(WORD) == 2);
C_ASSERT(sizeof(DWORD) == 4);
C_ASSERT(sizeof(QWORD) == 8);

C_ASSERT(sizeof(ULONG_PTR) == sizeof(void *));
C_ASSERT(sizeof(LONG_PTR) == sizeof(void *));

#endif /* __INTERNAL_TYPES_H__ */
