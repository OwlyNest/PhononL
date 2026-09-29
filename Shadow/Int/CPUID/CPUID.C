/*
	* Shadow/Int/CPUID/CPUID.C - CPU identification
	* Author:   amity
	* Date:     Tue Sep 29 15:29:27 2026
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

/* --- Includes ---*/
#include <XAL/XScope.H>
#include <Int/CPUID/CPUID.H>
#include <Lib/Lib.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _CpuidRawDb Db;
static _CpuInfo    Info;                            

static const UINT32 SimpleBasic[] = {
    0x00000001, 0x00000002, 0x00000003,
    0x00000005, 0x00000006,
    0x00000015, 0x00000016
};

static const UINT32 SimpleExtended[] = {
    0x80000001,
    0x80000002,
    0x80000003,
    0x80000004,
    0x80000005,
    0x80000006,
    0x80000007,
    0x80000008,
    0x8000000A,
    0x80000019,
    0x8000001A,
    0x8000001B,
    0x8000001C,
    // Special
    0x8000001E,
    0x8000001F,
    0x80000020,
    0x80000021,
};

/* --- Prototypes ---*/
static inline VOID CpuidExec(_PCpuidRawDb Db, UINT32 Leaf, UINT32 Subleaf);
static inline VOID U32ToBytes(UINT32 Val, CHAR *Buf);

/* --- Functions ---*/

/* ==========================================================================
 *                                                                          *
 * CpuidAvailable()                                                         *
 *                                                                          *
 * Check if the CPUID instruction is supported                              *
 * CPUID is supported if we can flip bit 21 (the ID bit) in EFLAGS          *
 * Returns non-zero if CPUID is available, zero otherwise                   *
 *                                                                          *
 ========================================================================== */
static INT CpuidAvailable(VOID) {
    UINT64 Orig, Mod;

    __asm__ __volatile__(
        "pushfq\r\n\t"                 // Save RFLAGS
        "popq %0\r\n\t"                // Orig = RFLAGS
        "movq %0, %1\r\n\t"            // Mod = Orig
        "xorq $0x00200000, %1\r\n\t"   // Flip ID bit (bit 21)
        "pushq %1\r\n\t"               // Push modified flags
        "popfq\r\n\t"                  // Load modified RFLAGS
        "pushfq\r\n\t"                 // Store RFLAGS again
        "popq %1\r\n\t"                // Mod = new RFLAGS
        "pushq %0\r\n\t"               // Push original flags
        "popfq\r\n\t"                  // Restore original RFLAGS
        "xorq %0, %1"                // Mod = changed bits
        : "=r"(Orig), "=r"(Mod)
        :
        : "cc", "memory");

    return (Mod & 0x00200000) != 0;
}

/* ==========================================================================
 *                                                                          *
 * CpuidRawFind()                                                           *
 *                                                                          *
 * Find raw CPUID data by leaf and subleaf                                  *
 *                                                                          *
 ========================================================================== */
static const _CpuidRaw *CpuidRawFind(const _CpuidRawDb *Db, UINT32 Leaf, UINT32 Subleaf) {
    for (UINT32 I = 0; I < Db->Count; I++) {
        _CpuidRaw Raw = Db->Entries[I];
        if (Raw.Leaf == Leaf && Raw.Subleaf == Subleaf) {
            return &Db->Entries[I];
        }
    }
    return NULL;
}

/* ==========================================================================
 *                                                                          *
 * CpuidExec()                                                              *
 *                                                                          *
 * Execute CPUID with EAX=leaf, ECX=subleaf and return all four registers   *
 *                                                                          *
 ========================================================================== */
static inline VOID CpuidExec(_PCpuidRawDb Db, UINT32 Leaf, UINT32 Subleaf) {
    UINT32 InSubleaf = Subleaf;
    UINT32 Ra, Rb, Rc, Rd;
    Rc = Subleaf;

    __asm__ __volatile__(
        "pushq %%rbx\r\n\t"
        "cpuid\r\n\t"
        "movl %%ebx, %1\r\n\t"
        "popq %%rbx"
        : "=a"(Ra), "=r"(Rb), "+c"(Rc), "=d"(Rd)
        : "a"(Leaf)
        : "cc", "memory");

    Db->Entries[Db->Count++] = (_CpuidRaw){
        .Leaf = Leaf,
        .Subleaf = InSubleaf,
        .Eax = Ra,
        .Ebx = Rb,
        .Ecx = Rc,
        .Edx = Rd
    };
}

/* ==========================================================================
 *                                                                          *
 * U32ToBytes()                                                             *
 *                                                                          *
 * Convert a little-endian UINT32 to 4 CHAR's                                *
 *                                                                          *
 ========================================================================== */
static inline VOID U32ToBytes(UINT32 Val, CHAR *Buf) {
    Buf[0] = (CHAR)(Val & 0xFF);
    Buf[1] = (CHAR)((Val >> 8) & 0xFF);
    Buf[2] = (CHAR)((Val >> 16) & 0xFF);
    Buf[3] = (CHAR)((Val >> 24) & 0xFF);
}

/* ==========================================================================
 *                                                                          *
 * CpuidRawPass()                                                           *
 *                                                                          *
 * Retrieve raw CPUID data                                                  *
 *                                                                          *
 ========================================================================== */
static VOID CpuidRawPass(VOID) {
    CpuidExec(&Db, 0x00000000, 0);
    UINT32 MaxBasic = CpuidRawFind(&Db, 0, 0)->Eax;
    for (SIZE_T I = 0; I < ARRAY_SIZE(SimpleBasic); I++) {
        if (MaxBasic >= SimpleBasic[I]) {
            CpuidExec(&Db, SimpleBasic[I], 0);
        }
    }

    for (UINT32 Idx = 0;; Idx++) {
        CpuidExec(&Db, 4, Idx);

        if ((CpuidRawFind(&Db, 4, Idx)->Eax & 0x1F) == 0) {
            break;
        }
    }

    CpuidExec(&Db, 0x00000007, 0);
    for (UINT32 Idx = 1; Idx <= CpuidRawFind(&Db, 7, 0)->Eax; Idx++) {
        CpuidExec(&Db, 7, Idx);
    }

    for (UINT32 Idx = 0;; Idx++) {
        CpuidExec(&Db, 0x0B, Idx);

        UINT32 Ecx = CpuidRawFind(&Db, 0x0B, Idx)->Ecx;
        UINT32 LevelType = (Ecx >> 8) & 0xFF;

        if (LevelType == 0) {
            break;
        }
    }

    CpuidExec(&Db, 0x0D, 0);
    CpuidExec(&Db, 0x0D, 1);
    const _CpuidRaw *Leaf = CpuidRawFind(&Db, 0x0D, 0);
    UINT64 Bitmap = ((UINT64)Leaf->Edx << 32) | Leaf->Eax;
    for (UINT32 I = 2; I < 64; I++) {
        if (Bitmap & (1ULL << I)) {
            CpuidExec(&Db, 0x0D, I);
        }
    }

    CpuidExec(&Db, 0x80000000, 0);
    UINT32 MaxExtended = CpuidRawFind(&Db, 0x80000000, 0)->Eax;
    for (SIZE_T I = 0; I < ARRAY_SIZE(SimpleExtended); I++) {
        if (MaxExtended >= SimpleExtended[I]) {
            CpuidExec(&Db, SimpleExtended[I], 0);
        }
    }

    if (MaxExtended >= 0x8000001D) {
        for (UINT32 Idx = 0;; Idx++) {
            CpuidExec(&Db, 0x8000001D, Idx);

            if ((CpuidRawFind(&Db, 0x8000001D, Idx)->Eax & 0x1F) == 0) {
                break;
            }
        }
    }
}

/* ==========================================================================
 *                                                                          *
 * CpuidDumpDb()                                                            *
 *                                                                          *
 * Dump raw database in a formatted table                                   *
 *                                                                          *
 ========================================================================== */
static VOID CpuidDumpDb(VOID) {
    /*
     * Don't order just dump
     * User can read, I think, probably
     */
    printk("Leaf       Sub EAX        EBX        ECX        EDX\r\n");
    for (UINT32 Idx = 0; Idx < Db.Count; Idx++) {
        const _CpuidRaw *Leaf = &Db.Entries[Idx];
        printk("%#010x %-3u %#010x %#010x %#010x %#010x\r\n",
               Leaf->Leaf, Leaf->Subleaf,
               Leaf->Eax, Leaf->Ebx, Leaf->Ecx, Leaf->Edx);
    }
}

static VOID CpuidDecodeVendor(_PCpuidRawDb Db, _PCpuInfo Info) {
    const _CpuidRaw *Leaf = CpuidRawFind(Db, 0, 0);

    U32ToBytes(Leaf->Ebx, (CHAR *)&Info->Vendor[0]);
    U32ToBytes(Leaf->Edx, (CHAR *)&Info->Vendor[4]);
    U32ToBytes(Leaf->Ecx, (CHAR *)&Info->Vendor[8]);
    Info->Vendor[12] = '\0';
}

static VOID CpuidDecodeBrand(_PCpuidRawDb Db, _PCpuInfo Info) {
    Info->Brand[0] = '\0';

    const _CpuidRaw *Leaf = CpuidRawFind(Db, 0x80000000, 0);

    if (Leaf->Eax < 0x80000004) {
        return; // Brand string not supported
    }

    for (INT I = 0; I < 3; I++) {
        const _CpuidRaw *BrandLeaf = CpuidRawFind(Db, 0x80000002 + I, 0);

        U32ToBytes(BrandLeaf->Eax, (CHAR *)&Info->Brand[I * 16 + 0]);
        U32ToBytes(BrandLeaf->Ebx, (CHAR *)&Info->Brand[I * 16 + 4]);
        U32ToBytes(BrandLeaf->Ecx, (CHAR *)&Info->Brand[I * 16 + 8]);
        U32ToBytes(BrandLeaf->Edx, (CHAR *)&Info->Brand[I * 16 + 12]);
    }
    Info->Brand[48] = '\0';
}

static VOID CpuidDecodePsn(_PCpuidRawDb Db, _PCpuInfo Info) {
    const _CpuidRaw *Leaf1 = CpuidRawFind(Db, 1, 0);
    const _CpuidRaw *Leaf3 = CpuidRawFind(Db, 3, 0);
    Info->Psn[0] = Leaf1->Eax;
    Info->Psn[1] = Leaf3->Ecx;
    Info->Psn[2] = Leaf3->Edx;
}

static VOID CpuidDecodeProc(_PCpuidRawDb Db, _PCpuidProcInfo Info) {
    const _CpuidRaw *Leaf1 = CpuidRawFind(Db, 1, 0);

    Info->Stepping = (BYTE)(Leaf1->Eax & 0x0F);
    Info->Model = (BYTE)((Leaf1->Eax >> 4) & 0x0F);
    Info->Family = (BYTE)((Leaf1->Eax >> 8) & 0x0F);
    Info->ProcType = (BYTE)((Leaf1->Eax >> 12) & 0x03);
    Info->ExtModel = (BYTE)((Leaf1->Eax >> 16) & 0x0F);
    Info->ExtFamily = (BYTE)((Leaf1->Eax >> 20) & 0xFF);

    Info->BrandIndex = (BYTE)(Leaf1->Ebx & 0xFF);
    Info->ClflushLineSize = (BYTE)(((Leaf1->Ebx >> 8) & 0xFF) * 8);
    Info->MaxLogicalIds = (BYTE)((Leaf1->Ebx >> 16) & 0xFF);
    Info->InitialApicId = (Leaf1->Ebx >> 24) & 0xFF;

    // Intel/AMD display rules: if family == 6 or 15, display model
    // is (ext_model << 4) + model. If family == 15, display family
    // is ext_family + family.
    UINT32 Family = Info->Family;
    UINT32 Model = Info->Model;

    if (Family == 0x06 || Family == 0x0F) {
        Model = (Info->ExtModel << 4) | Info->Model;
    }
    if (Family == 0x0F) {
        Family = Info->ExtFamily + Info->Family;
    }

    Info->DisplayModel = (BYTE)Model;
    Info->DisplayFamily = (BYTE)Family;
}

static VOID CpuidDecodeFeat(_PCpuidRawDb Db, _PCpuidFeat Info) {
    const _CpuidRaw *Leaf = CpuidRawFind(Db, 1, 0);

    UINT32 Ecx = Leaf->Ecx;
    UINT32 Edx = Leaf->Edx;

    Info->Fpu = (BYTE)((Edx >> 0) & 0x01);
    Info->Vme = (BYTE)((Edx >> 1) & 0x01);
    Info->De = (BYTE)((Edx >> 2) & 0x01);
    Info->Pse = (BYTE)((Edx >> 3) & 0x01);
    Info->Tsc = (BYTE)((Edx >> 4) & 0x01);
    Info->Msr = (BYTE)((Edx >> 5) & 0x01);
    Info->Pae = (BYTE)((Edx >> 6) & 0x01);
    Info->Mce = (BYTE)((Edx >> 7) & 0x01);
    Info->Cx8 = (BYTE)((Edx >> 8) & 0x01);
    Info->Apic = (BYTE)((Edx >> 9) & 0x01);

    Info->Sep = (BYTE)((Edx >> 11) & 0x01);
    Info->Mtrr = (BYTE)((Edx >> 12) & 0x01);
    Info->Pge = (BYTE)((Edx >> 13) & 0x01);
    Info->Mca = (BYTE)((Edx >> 14) & 0x01);
    Info->Cmov = (BYTE)((Edx >> 15) & 0x01);
    Info->Pat = (BYTE)((Edx >> 16) & 0x01);
    Info->Pse36 = (BYTE)((Edx >> 17) & 0x01);
    Info->Psn = (BYTE)((Edx >> 18) & 0x01);
    Info->Clfsh = (BYTE)((Edx >> 19) & 0x01);
    Info->Nx = (BYTE)((Edx >> 20) & 0x01);
    Info->Ds = (BYTE)((Edx >> 21) & 0x01);
    Info->Acpi = (BYTE)((Edx >> 22) & 0x01);
    Info->Mmx = (BYTE)((Edx >> 23) & 0x01);
    Info->Fxsr = (BYTE)((Edx >> 24) & 0x01);
    Info->Sse = (BYTE)((Edx >> 25) & 0x01);
    Info->Sse2 = (BYTE)((Edx >> 26) & 0x01);
    Info->Ss = (BYTE)((Edx >> 27) & 0x01);
    Info->Htt = (BYTE)((Edx >> 28) & 0x01);
    Info->Tm = (BYTE)((Edx >> 29) & 0x01);
    Info->Ia64 = (BYTE)((Edx >> 30) & 0x01);
    Info->Pbe = (BYTE)((Edx >> 31) & 0x01);

    Info->Sse3 = (BYTE)((Ecx >> 0) & 0x01);
    Info->Pclmulqdq = (BYTE)((Ecx >> 1) & 0x01);
    Info->Dtes64 = (BYTE)((Ecx >> 2) & 0x01);
    Info->Monitor = (BYTE)((Ecx >> 3) & 0x01);
    Info->DsCpl = (BYTE)((Ecx >> 4) & 0x01);
    Info->Vmx = (BYTE)((Ecx >> 5) & 0x01);
    Info->Smx = (BYTE)((Ecx >> 6) & 0x01);
    Info->Est = (BYTE)((Ecx >> 7) & 0x01);
    Info->Tm2 = (BYTE)((Ecx >> 8) & 0x01);
    Info->Ssse3 = (BYTE)((Ecx >> 9) & 0x01);
    Info->CntxId = (BYTE)((Ecx >> 10) & 0x01);
    Info->Sdbg = (BYTE)((Ecx >> 11) & 0x01);
    Info->Fma = (BYTE)((Ecx >> 12) & 0x01);
    Info->Cx16 = (BYTE)((Ecx >> 13) & 0x01);
    Info->Xtpr = (BYTE)((Ecx >> 14) & 0x01);
    Info->Pdcm = (BYTE)((Ecx >> 15) & 0x01);
    // reserved
    Info->Pcid = (BYTE)((Ecx >> 17) & 0x01);
    Info->Dca = (BYTE)((Ecx >> 18) & 0x01);
    Info->Sse41 = (BYTE)((Ecx >> 19) & 0x01);
    Info->Sse42 = (BYTE)((Ecx >> 20) & 0x01);
    Info->X2apic = (BYTE)((Ecx >> 21) & 0x01);
    Info->Movbe = (BYTE)((Ecx >> 22) & 0x01);
    Info->Popcnt = (BYTE)((Ecx >> 23) & 0x01);
    Info->TscDeadline = (BYTE)((Ecx >> 24) & 0x01);
    Info->AesNi = (BYTE)((Ecx >> 25) & 0x01);
    Info->Xsave = (BYTE)((Ecx >> 26) & 0x01);
    Info->Osxsave = (BYTE)((Ecx >> 27) & 0x01);
    Info->Avx = (BYTE)((Ecx >> 28) & 0x01);
    Info->F16c = (BYTE)((Ecx >> 29) & 0x01);
    Info->Rdrnd = (BYTE)((Ecx >> 30) & 0x01);
    Info->Hypervisor = (BYTE)((Ecx >> 31) & 0x01);
}

/* ==========================================================================
 *                                                                          *
 * CpuidDecodeExtFeat()                                                     *
 *                                                                          *
 * Decode Extended features from leaf 0x80000001                            *
 *                                                                          *
 ========================================================================== */
static VOID CpuidDecodeExtFeat(_PCpuidRawDb Db, _PCpuidExtFeat Info) {
    UINT32 Ecx, Edx;

    const _CpuidRaw *Leaf = CpuidRawFind(Db, 0x80000001, 0);
    Ecx = Leaf->Ecx;
    Edx = Leaf->Edx;

    Info->Fpu = (BYTE)((Edx >> 0) & 0x01);
    Info->Vme = (BYTE)((Edx >> 1) & 0x01);
    Info->De = (BYTE)((Edx >> 2) & 0x01);
    Info->Pse = (BYTE)((Edx >> 3) & 0x01);
    Info->Tsc = (BYTE)((Edx >> 4) & 0x01);
    Info->Msr = (BYTE)((Edx >> 5) & 0x01);
    Info->Pae = (BYTE)((Edx >> 6) & 0x01);
    Info->Mce = (BYTE)((Edx >> 7) & 0x01);
    Info->Cx8 = (BYTE)((Edx >> 8) & 0x01);
    Info->Apic = (BYTE)((Edx >> 9) & 0x01);
    Info->SyscallK6 = (BYTE)((Edx >> 10) & 0x01);
    Info->Syscall = (BYTE)((Edx >> 11) & 0x01);
    Info->Mtrr = (BYTE)((Edx >> 12) & 0x01);
    Info->Pge = (BYTE)((Edx >> 13) & 0x01);
    Info->Mca = (BYTE)((Edx >> 14) & 0x01);
    Info->Cmov = (BYTE)((Edx >> 15) & 0x01);
    Info->Pat = (BYTE)((Edx >> 16) & 0x01);
    Info->Pse36 = (BYTE)((Edx >> 17) & 0x01);
    Info->EccK7 = (BYTE)((Edx >> 18) & 0x01);
    Info->Ecc = (BYTE)((Edx >> 19) & 0x01);
    Info->Nx = (BYTE)((Edx >> 20) & 0x01);
    Info->Sem = (BYTE)((Edx >> 21) & 0x01);
    Info->Mmxext = (BYTE)((Edx >> 22) & 0x01);
    Info->Mmx = (BYTE)((Edx >> 23) & 0x01);
    Info->Fxsr = (BYTE)((Edx >> 24) & 0x01);
    Info->FxsrOpt = (BYTE)((Edx >> 25) & 0x01);
    Info->Pdpe1gb = (BYTE)((Edx >> 26) & 0x01);
    Info->Rdtscp = (BYTE)((Edx >> 27) & 0x01);
    Info->Rex32K8 = (BYTE)((Edx >> 28) & 0x01);
    Info->Lm = (BYTE)((Edx >> 29) & 0x01);
    Info->Tdnowext = (BYTE)((Edx >> 30) & 0x01);
    Info->Tdnow = (BYTE)((Edx >> 31) & 0x01);

    Info->LahfLm = (BYTE)((Ecx >> 0) & 0x01);
    Info->CmpLegacy = (BYTE)((Ecx >> 1) & 0x01);
    Info->Svm = (BYTE)((Ecx >> 2) & 0x01);
    Info->Extapic = (BYTE)((Ecx >> 3) & 0x01);
    Info->Cr8Legacy = (BYTE)((Ecx >> 4) & 0x01);
    Info->Abm = (BYTE)((Ecx >> 5) & 0x01);
    Info->Sse4a = (BYTE)((Ecx >> 6) & 0x01);
    Info->Misalignsse = (BYTE)((Ecx >> 7) & 0x01);
    Info->Tdnowprefetch = (BYTE)((Ecx >> 8) & 0x01);
    Info->Osvw = (BYTE)((Ecx >> 9) & 0x01);
    Info->Ibs = (BYTE)((Ecx >> 10) & 0x01);
    Info->Xop = (BYTE)((Ecx >> 11) & 0x01);
    Info->Skinit = (BYTE)((Ecx >> 12) & 0x01);
    Info->Wdt = (BYTE)((Ecx >> 13) & 0x01);
    Info->Tbm0 = (BYTE)((Ecx >> 14) & 0x01);
    Info->Lwp = (BYTE)((Ecx >> 15) & 0x01);
    Info->Fma4 = (BYTE)((Ecx >> 16) & 0x01);
    Info->Tce = (BYTE)((Ecx >> 17) & 0x01);
    Info->Cvt16 = (BYTE)((Ecx >> 18) & 0x01);
    Info->NodeidMsr = (BYTE)((Ecx >> 19) & 0x01);
    // reserved
    Info->Tbm = (BYTE)((Ecx >> 21) & 0x01);
    Info->Topoext = (BYTE)((Ecx >> 22) & 0x01);
    Info->PerfctrCore = (BYTE)((Ecx >> 23) & 0x01);
    Info->PerfctrNb = (BYTE)((Ecx >> 24) & 0x01);
    Info->StreamPerfMon = (BYTE)((Ecx >> 25) & 0x01);
    Info->Dbx = (BYTE)((Ecx >> 26) & 0x01);
    Info->Perftsc = (BYTE)((Ecx >> 27) & 0x01);
    Info->PcxL2iL3 = (BYTE)((Ecx >> 28) & 0x01);
    Info->Monitorx = (BYTE)((Ecx >> 29) & 0x01);
    Info->AddrMaskExt = (BYTE)((Ecx >> 30) & 0x01);
    // reserved
}

static VOID CpuidDecodeFeat7(_PCpuidRawDb Db, _PCpuidFeat7 Info) {
    UINT32 Eax, Ebx, Ecx, Edx;
    /* --- Sub Leaf 0 --- */
    const _CpuidRaw *Leaf70 = CpuidRawFind(Db, 7, 0);
    Ebx = Leaf70->Ebx;
    Ecx = Leaf70->Ecx;
    Edx = Leaf70->Edx;
    // EBX
    Info->Fsgsbase = (BYTE)((Ebx >> 0) & 0x01);
    Info->TscAdjust = (BYTE)((Ebx >> 1) & 0x01);
    Info->Sgx = (BYTE)((Ebx >> 2) & 0x01);
    Info->Bmi1 = (BYTE)((Ebx >> 3) & 0x01);
    Info->Hle = (BYTE)((Ebx >> 4) & 0x01);
    Info->Avx2 = (BYTE)((Ebx >> 5) & 0x01);
    Info->FdpExcptnOnly = (BYTE)((Ebx >> 6) & 0x01);
    Info->Smep = (BYTE)((Ebx >> 7) & 0x01);
    Info->Bmi2 = (BYTE)((Ebx >> 8) & 0x01);
    Info->Erms = (BYTE)((Ebx >> 9) & 0x01);
    Info->Invpcid = (BYTE)((Ebx >> 10) & 0x01);
    Info->Rtm = (BYTE)((Ebx >> 11) & 0x01);
    Info->RdtMPqm = (BYTE)((Ebx >> 12) & 0x01);
    Info->FcsFdsDeprecation = (BYTE)((Ebx >> 13) & 0x01);
    Info->Mpx = (BYTE)((Ebx >> 14) & 0x01);
    Info->RdtAPqe = (BYTE)((Ebx >> 15) & 0x01);
    Info->Avx512F = (BYTE)((Ebx >> 16) & 0x01);
    Info->Avx512Dq = (BYTE)((Ebx >> 17) & 0x01);
    Info->Rdseed = (BYTE)((Ebx >> 18) & 0x01);
    Info->Adx = (BYTE)((Ebx >> 19) & 0x01);
    Info->Smap = (BYTE)((Ebx >> 20) & 0x01);
    Info->Avx512Ifma = (BYTE)((Ebx >> 21) & 0x01);
    Info->Pmcommit = (BYTE)((Ebx >> 22) & 0x01);
    Info->Clflushopt = (BYTE)((Ebx >> 23) & 0x01);
    Info->Clwb = (BYTE)((Ebx >> 24) & 0x01);
    Info->Pt = (BYTE)((Ebx >> 25) & 0x01);
    Info->Avx512Pf = (BYTE)((Ebx >> 26) & 0x01);
    Info->Avx512Er = (BYTE)((Ebx >> 27) & 0x01);
    Info->Avx512Cd = (BYTE)((Ebx >> 28) & 0x01);
    Info->Sha = (BYTE)((Ebx >> 29) & 0x01);
    Info->Avx512Bw = (BYTE)((Ebx >> 30) & 0x01);
    Info->Avx512Vl = (BYTE)((Ebx >> 31) & 0x01);
    // ECX
    Info->Prefetchwt1 = (BYTE)((Ecx >> 0) & 0x01);
    Info->Avx512Vbmi = (BYTE)((Ecx >> 1) & 0x01);
    Info->Umip = (BYTE)((Ecx >> 2) & 0x01);
    Info->Pku = (BYTE)((Ecx >> 3) & 0x01);
    Info->Ospke = (BYTE)((Ecx >> 4) & 0x01);
    Info->Waitpkg = (BYTE)((Ecx >> 5) & 0x01);
    Info->Avx512Vbmi2 = (BYTE)((Ecx >> 6) & 0x01);
    Info->CetSs = (BYTE)((Ecx >> 7) & 0x01);
    Info->Gfni = (BYTE)((Ecx >> 8) & 0x01);
    Info->Vaes = (BYTE)((Ecx >> 9) & 0x01);
    Info->Vpclmulqdq = (BYTE)((Ecx >> 10) & 0x01);
    Info->Avx512Vnni = (BYTE)((Ecx >> 11) & 0x01);
    Info->Avx512Bitalg = (BYTE)((Ecx >> 12) & 0x01);
    Info->TmeEn = (BYTE)((Ecx >> 13) & 0x01);
    Info->Avx512Vpopcntdq = (BYTE)((Ecx >> 14) & 0x01);
    Info->Fzm = (BYTE)((Ecx >> 15) & 0x01);
    Info->La57 = (BYTE)((Ecx >> 16) & 0x01);
    Info->Mawau = (DWORD)((Ecx >> 17) & 0x1F);
    Info->Rdpid = (BYTE)((Ecx >> 22) & 0x01);
    Info->Kl = (BYTE)((Ecx >> 23) & 0x01);
    Info->BusLockDetect = (BYTE)((Ecx >> 24) & 0x01);
    Info->Cldemote = (BYTE)((Ecx >> 25) & 0x01);
    Info->Mprr = (BYTE)((Ecx >> 26) & 0x01);
    Info->Movdiri = (BYTE)((Ecx >> 27) & 0x01);
    Info->Movdir64b = (BYTE)((Ecx >> 28) & 0x01);
    Info->Enqcmd = (BYTE)((Ecx >> 29) & 0x01);
    Info->SgxLc = (BYTE)((Ecx >> 30) & 0x01);
    Info->Pks = (BYTE)((Ecx >> 31) & 0x01);
    // EDX
    Info->SgxTerm = (BYTE)((Edx >> 0) & 0x01);
    Info->SgxKeys = (BYTE)((Edx >> 1) & 0x01);
    Info->Avx512_4Vnniw = (BYTE)((Edx >> 2) & 0x01);
    Info->Avx512_4Fmaps = (BYTE)((Edx >> 3) & 0x01);
    Info->Fsrm = (BYTE)((Edx >> 4) & 0x01);
    Info->Uintr = (BYTE)((Edx >> 5) & 0x01);
    // reserved
    // reserved
    Info->Avx512Vp2intersect = (BYTE)((Edx >> 8) & 0x01);
    Info->SrbdsCtrl = (BYTE)((Edx >> 9) & 0x01);
    Info->MdClear = (BYTE)((Edx >> 10) & 0x01);
    Info->RtmAlwaysAbort = (BYTE)((Edx >> 11) & 0x01);
    // reserved
    Info->RtmForceAbort = (BYTE)((Edx >> 13) & 0x01);
    Info->Serialize = (BYTE)((Edx >> 14) & 0x01);
    Info->Hybrid = (BYTE)((Edx >> 15) & 0x01);
    Info->Tsxldtrk = (BYTE)((Edx >> 16) & 0x01);
    // reserved
    Info->Pconfig = (BYTE)((Edx >> 18) & 0x01);
    Info->Lbr = (BYTE)((Edx >> 19) & 0x01);
    Info->CetIbt = (BYTE)((Edx >> 20) & 0x01);
    // reserved
    Info->AmxBf16 = (BYTE)((Edx >> 22) & 0x01);
    Info->Avx512Fp16 = (BYTE)((Edx >> 23) & 0x01);
    Info->AmxTile = (BYTE)((Edx >> 24) & 0x01);
    Info->AmxInt8 = (BYTE)((Edx >> 25) & 0x01);
    Info->SpecCtrl = (BYTE)((Edx >> 26) & 0x01);
    Info->Stibp = (BYTE)((Edx >> 27) & 0x01);
    Info->L1dFlush = (BYTE)((Edx >> 28) & 0x01);
    Info->ArchCapabilities = (BYTE)((Edx >> 29) & 0x01);
    Info->CoreCapabilities = (BYTE)((Edx >> 30) & 0x01);
    Info->Ssbd = (BYTE)((Edx >> 31) & 0x01);
    /* --- Sub Leaf 1 --- */
    const _CpuidRaw *Leaf71 = CpuidRawFind(Db, 7, 1);
    if (Leaf71 == NULL) {
        return;
    }
    Eax = Leaf71->Eax;
    Ebx = Leaf71->Ebx;
    Ecx = Leaf71->Ecx;
    Edx = Leaf71->Edx;
    // EAX
    Info->Sha512 = (BYTE)((Eax >> 0) & 0x01);
    Info->Sm3 = (BYTE)((Eax >> 1) & 0x01);
    Info->Sm4 = (BYTE)((Eax >> 2) & 0x01);
    Info->RaoInt = (BYTE)((Eax >> 3) & 0x01);
    Info->AmxVnni = (BYTE)((Eax >> 4) & 0x01);
    Info->Avx512Bf16 = (BYTE)((Eax >> 5) & 0x01);
    Info->Lass = (BYTE)((Eax >> 6) & 0x01);
    Info->Cmpccxadd = (BYTE)((Eax >> 7) & 0x01);
    Info->Archperfmonext = (BYTE)((Eax >> 8) & 0x01);
    Info->Dedup = (BYTE)((Eax >> 9) & 0x01);
    Info->Fzrm = (BYTE)((Eax >> 10) & 0x01);
    Info->Fsrs = (BYTE)((Eax >> 11) & 0x01);
    Info->Rsrcs = (BYTE)((Eax >> 12) & 0x01);
    // reserved
    // reserved
    // reserved
    // reserved
    Info->Fred = (BYTE)((Eax >> 17) & 0x01);
    Info->Lkgs = (BYTE)((Eax >> 18) & 0x01);
    Info->Wrmsrns = (BYTE)((Eax >> 19) & 0x01);
    Info->NmiSrc = (BYTE)((Eax >> 20) & 0x01);
    Info->AmxFp16 = (BYTE)((Eax >> 21) & 0x01);
    Info->Hreset = (BYTE)((Eax >> 22) & 0x01);
    Info->AvxIfma = (BYTE)((Eax >> 23) & 0x01);
    // reserved
    // reserved
    Info->Lam = (BYTE)((Eax >> 26) & 0x01);
    Info->Msrlist = (BYTE)((Eax >> 27) & 0x01);
    // reserved
    // reserved
    Info->InvdDisablePostBiosDone = (BYTE)((Eax >> 30) & 0x01);
    Info->Movrs = (BYTE)((Eax >> 31) & 0x01);
    // EBX
    Info->Ppin = (BYTE)((Ebx >> 0) & 0x01);
    Info->Pbndkb = (BYTE)((Ebx >> 1) & 0x01);
    // reserved
    Info->CpuidMaxvalLimRmv = (BYTE)((Ebx >> 3) & 0x01);
    // reserved ...
    Info->Mpsadbw512 = (BYTE)((Ebx >> 28) & 0x01);
    // reserved
    Info->Avx512RaoFp = (BYTE)((Ebx >> 30) & 0x01);
    // ECX
    Info->RdtMAsym = (BYTE)((Ecx >> 0) & 0x01);
    Info->RdtAAsym = (BYTE)((Ecx >> 1) & 0x01);
    Info->ReducedIsa = (BYTE)((Ecx >> 2) & 0x01);
    // reserved
    Info->Sipi64 = (BYTE)((Ecx >> 4) & 0x01);
    Info->MsrImm = (BYTE)((Ecx >> 5) & 0x01);
    // reserved
    Info->Ace = (BYTE)((Ecx >> 11) & 0x01);
    // EDX
    // reserved
    Info->Avx512VnniFp16 = (BYTE)((Edx >> 1) & 0x01);
    Info->Avx512VnniInt8 = (BYTE)((Edx >> 2) & 0x01);
    Info->Avx512NeConvert = (BYTE)((Edx >> 3) & 0x01);
    Info->AvxVnniInt8 = (BYTE)((Edx >> 4) & 0x01);
    Info->AvxNeConvert = (BYTE)((Edx >> 5) & 0x01);
    // reserved
    // reserved
    Info->AmxComplex = (BYTE)((Edx >> 8) & 0x01);
    // reserved
    Info->AvxVnniInt16 = (BYTE)((Edx >> 10) & 0x01);
    Info->Avx512VnniInt16 = (BYTE)((Edx >> 11) & 0x01);
    // reserved
    Info->Utmr = (BYTE)((Edx >> 13) & 0x01);
    Info->Prefetchi = (BYTE)((Edx >> 14) & 0x01);
    Info->UserMsr = (BYTE)((Edx >> 15) & 0x01);
    Info->Avx512Bf16Ne = (BYTE)((Edx >> 16) & 0x01);
    Info->UiretUifFromRflags = (BYTE)((Edx >> 17) & 0x01);
    Info->CetSss = (BYTE)((Edx >> 18) & 0x01);
    Info->Avx10 = (BYTE)((Edx >> 19) & 0x01);
    // reserved
    Info->ApxF = (BYTE)((Edx >> 21) & 0x01);
    Info->SecTeeAttestation = (BYTE)((Edx >> 22) & 0x01);
    Info->Mwait = (BYTE)((Edx >> 23) & 0x01);
    Info->Slsm = (BYTE)((Edx >> 24) & 0x01);
    /* --- Sub Leaf 2 --- */
    const _CpuidRaw *Leaf72 = CpuidRawFind(Db, 7, 2);
    if (Leaf72 == NULL) {
        return;
    }
    Edx = Leaf72->Edx;
    // EDX
    Info->Psfd = (BYTE)((Edx >> 0) & 0x01);
    Info->IpredCtrl = (BYTE)((Edx >> 1) & 0x01);
    Info->RrsbaCtrl = (BYTE)((Edx >> 2) & 0x01);
    Info->DdpuU = (BYTE)((Edx >> 3) & 0x01);
    Info->BhiCtrl = (BYTE)((Edx >> 4) & 0x01);
    Info->McdtNo = (BYTE)((Edx >> 5) & 0x01);
    Info->UcLockDisable = (BYTE)((Edx >> 6) & 0x01);
    Info->MonitorMitgNo = (BYTE)((Edx >> 7) & 0x01);
}

/* ==========================================================================
 *                                                                          *
 * CpuidDecode()                                                            *
 *                                                                          *
 * Decode CPUID leaves                                                      *
 *                                                                          *
 ========================================================================== */
static VOID CpuidDecode(_PCpuidRawDb Db, _PCpuInfo Info) {
    CpuidDecodeVendor(Db, Info);
    CpuidDecodeBrand(Db, Info);
    CpuidDecodePsn(Db, Info);
    CpuidDecodeProc(Db, &Info->Proc);
    CpuidDecodeFeat(Db, &Info->Features);
    CpuidDecodeExtFeat(Db, &Info->FeaturesExt);
    CpuidDecodeFeat7(Db, &Info->Feat7);
}

/* ==========================================================================
 *                                                                          *
 * CpuidInit()                                                              *
 *                                                                          *
 * Public entry: collect raw leaves and decode into caller-owned Info       *
 *                                                                          *
 ========================================================================== */
static SHSTATUS CpuidInit(VOID) {
	if (!CpuidAvailable()) {
		return STATUS_SUCCESS;
	}
    CpuidRawPass();
    CpuidDumpDb();
    CpuidDecode(&Db, &Info);
	return STATUS_SUCCESS;
}

XSCOPENODE_PRI(CPUID, CpuidInit, XSCOPE_PRIORITY_FIRST, "SERIAL");

#define PRINT_FEAT(Field, Desc)                                                \
    do {                                                                       \
        if (Feat->Field)                                                       \
            printk("  " Desc "\r\n");                                            \
    } while (0)

static VOID DumpFeat(_PCpuidFeat Feat) {
    printk("\r\nFeatures\r\n");

    PRINT_FEAT(Fpu, "Onboard x87 FPU");
    PRINT_FEAT(Vme, "Virtual mode extensions");
    PRINT_FEAT(De, "Debugging extensions (CR4 bit 3)");
    PRINT_FEAT(Pse, "Page Size Extension");
    PRINT_FEAT(Tsc, "Time Stamp Counter");
    PRINT_FEAT(Msr, "Model-specific registers");
    PRINT_FEAT(Pae, "Physical Address Extension");
    PRINT_FEAT(Mce, "Machine Check Exception");
    PRINT_FEAT(Cx8, "Compare-and-exchange 8B instruction");
    PRINT_FEAT(Apic, "Onboard APIC");

    PRINT_FEAT(Sep, "SYSENTER/SYSEXIT");
    PRINT_FEAT(Mtrr, "Memory Type Range Registers");
    PRINT_FEAT(Pge, "Page Global Enable");
    PRINT_FEAT(Mca, "Machine Check Architecture");
    PRINT_FEAT(Cmov, "CMOV/FCMOV instructions");
    PRINT_FEAT(Pat, "Page Attribute Table");
    PRINT_FEAT(Pse36, "36-bit Page Size Extension");
    PRINT_FEAT(Psn, "Processor Serial Number");
    PRINT_FEAT(Clfsh, "CLFLUSH instruction");
    PRINT_FEAT(Nx, "NX bit");
    PRINT_FEAT(Ds, "Debug Store");
    PRINT_FEAT(Acpi, "Thermal control MSRs");
    PRINT_FEAT(Mmx, "MMX");
    PRINT_FEAT(Fxsr, "FXSAVE/FXRSTOR");
    PRINT_FEAT(Sse, "SSE");
    PRINT_FEAT(Sse2, "SSE2");
    PRINT_FEAT(Ss, "Self-snoop");
    PRINT_FEAT(Htt, "Hyper-Threading");
    PRINT_FEAT(Tm, "Thermal Monitor");
    PRINT_FEAT(Ia64, "IA64 emulation");
    PRINT_FEAT(Pbe, "Pending Break Enable");
    // ECX
    PRINT_FEAT(Sse3, "SSE3");
    PRINT_FEAT(Pclmulqdq, "PCLMULQDQ");
    PRINT_FEAT(Dtes64, "64-bit Debug Store");
    PRINT_FEAT(Monitor, "MONITOR/MWAIT");
    PRINT_FEAT(DsCpl, "CPL-qualified Debug Store");
    PRINT_FEAT(Vmx, "Intel VT-x");
    PRINT_FEAT(Smx, "Safer Mode Extensions");
    PRINT_FEAT(Est, "Enhanced SpeedStep");
    PRINT_FEAT(Tm2, "Thermal Monitor 2");
    PRINT_FEAT(Ssse3, "SSSE3");
    PRINT_FEAT(CntxId, "L1 Context ID");
    PRINT_FEAT(Sdbg, "Silicon Debug");
    PRINT_FEAT(Fma, "FMA");
    PRINT_FEAT(Cx16, "Compare-and-exchange 16B instruction");
    PRINT_FEAT(Xtpr, "xTPR Update Control");
    PRINT_FEAT(Pdcm, "PerfMon & Debug Capability");

    PRINT_FEAT(Pcid, "Process Context Identifiers");
    PRINT_FEAT(Dca, "Direct Cache Access");
    PRINT_FEAT(Sse41, "SSE4.1");
    PRINT_FEAT(Sse42, "SSE4.2");
    PRINT_FEAT(X2apic, "x2APIC");
    PRINT_FEAT(Movbe, "MOVBE");
    PRINT_FEAT(Popcnt, "POPCNT");
    PRINT_FEAT(TscDeadline, "TSC Deadline Timer");
    PRINT_FEAT(AesNi, "AES-NI");
    PRINT_FEAT(Xsave, "XSAVE");
    PRINT_FEAT(Osxsave, "OSXSAVE");
    PRINT_FEAT(Avx, "AVX");
    PRINT_FEAT(F16c, "F16C");
    PRINT_FEAT(Rdrnd, "RDRAND");
    PRINT_FEAT(Hypervisor, "Hypervisor Present");
}

static VOID DumpExtFeat(_PCpuidExtFeat Feat) {
    printk("\r\nExtended features\r\n");

    // EDX
    PRINT_FEAT(Fpu, "Onboard x87 FPU");
    PRINT_FEAT(Vme, "Virtual mode extensions");
    PRINT_FEAT(De, "Debugging extensions (CR4 bit 3)");
    PRINT_FEAT(Pse, "Page Size Extension");
    PRINT_FEAT(Tsc, "Time Stamp Counter");
    PRINT_FEAT(Msr, "Model-specific registers");
    PRINT_FEAT(Pae, "Physical Address Extension");
    PRINT_FEAT(Mce, "Machine Check Exception");
    PRINT_FEAT(Cx8, "compare-and-swap instruction");
    PRINT_FEAT(Apic, "Onboard APIC");
    PRINT_FEAT(SyscallK6, "SYSCALL/SYSRET (K6)");
    PRINT_FEAT(Syscall, "SYSCALL/SYSRET");
    PRINT_FEAT(Mtrr, "Memory Type Range Registers");
    PRINT_FEAT(Pge, "Page Global Enable");
    PRINT_FEAT(Mca, "Machine Check Architecture");
    PRINT_FEAT(Cmov, "CMOV/FCMOV instructions");
    PRINT_FEAT(Pat, "Page Attribute Table");
    PRINT_FEAT(Pse36, "36-bit Page Size Extension");
    PRINT_FEAT(EccK7, "ECC (K7)");
    PRINT_FEAT(Ecc, "ECC");
    PRINT_FEAT(Nx, "NX bit");
    PRINT_FEAT(Sem, "SEM (AMD legacy feature)");
    PRINT_FEAT(Mmxext, "Extended MMX");
    PRINT_FEAT(Mmx, "MMX");
    PRINT_FEAT(Fxsr, "FXSAVE/FXRSTOR");
    PRINT_FEAT(FxsrOpt, "FXSAVE/FXRSTOR optimizations");
    PRINT_FEAT(Pdpe1gb, "1 GiB Pages");
    PRINT_FEAT(Rdtscp, "RDTSCP");
    PRINT_FEAT(Rex32K8, "REX prefix (K8)");
    PRINT_FEAT(Lm, "Long Mode");
    PRINT_FEAT(Tdnowext, "Extended 3DNow!");
    PRINT_FEAT(Tdnow, "3DNow!");

    // ECX
    PRINT_FEAT(LahfLm, "LAHF/SAHF in Long Mode");
    PRINT_FEAT(CmpLegacy, "CMP Legacy");
    PRINT_FEAT(Svm, "Secure Virtual Machine");
    PRINT_FEAT(Extapic, "Extended APIC Space");
    PRINT_FEAT(Cr8Legacy, "CR8 in 32-bit Mode");
    PRINT_FEAT(Abm, "Advanced Bit Manipulation");
    PRINT_FEAT(Sse4a, "SSE4a");
    PRINT_FEAT(Misalignsse, "Misaligned SSE");
    PRINT_FEAT(Tdnowprefetch, "PREFETCH/PREFETCHW");
    PRINT_FEAT(Osvw, "OS Visible Workaround");
    PRINT_FEAT(Ibs, "Instruction Based Sampling");
    PRINT_FEAT(Xop, "XOP");
    PRINT_FEAT(Skinit, "SKINIT/STGI");
    PRINT_FEAT(Wdt, "Watchdog Timer");
    PRINT_FEAT(Tbm0, "TBM0");
    PRINT_FEAT(Lwp, "Lightweight Profiling");
    PRINT_FEAT(Fma4, "FMA4");
    PRINT_FEAT(Tce, "Translation Cache Extension");
    PRINT_FEAT(Cvt16, "FP16/FP32 Conversion (XOP)");
    PRINT_FEAT(NodeidMsr, "NodeID MSR");
    PRINT_FEAT(Tbm, "Trailing Bit Manipulation");
    PRINT_FEAT(Topoext, "Topology Extensions");
    PRINT_FEAT(PerfctrCore, "Core Performance Counter Extensions");
    PRINT_FEAT(PerfctrNb, "Northbridge Performance Counter Extensions");
    PRINT_FEAT(StreamPerfMon, "Streaming Performance Monitor");
    PRINT_FEAT(Dbx, "Data Breakpoint Extensions");
    PRINT_FEAT(Perftsc, "Performance Timestamp Counter");
    PRINT_FEAT(PcxL2iL3, "AMD L2i/L3 Performance Counters");
    PRINT_FEAT(Monitorx, "MONITORX/MWAITX");
    PRINT_FEAT(AddrMaskExt, "Address Mask Extension");

    printk("\r\n");
}

static VOID DumpFeat7(_PCpuidFeat7 Feat) {
    printk("\r\nL7 Features\r\n");
    if (Feat->Mawau != 0) {
        printk("  MPX Address-Width Adjust: 0x%x\r\n", Feat->Mawau);
    }
    PRINT_FEAT(Fsgsbase, "RDFSBASE/RDGSBASE/WRFSBASE/WRGSBASE");
    PRINT_FEAT(TscAdjust, "TSC Adjust MSR");
    PRINT_FEAT(Sgx, "Software Guard Extensions");
    PRINT_FEAT(Bmi1, "Bit Manipulation Instructions 1");
    PRINT_FEAT(Hle, "Hardware Lock Elision");
    PRINT_FEAT(Avx2, "AVX2");
    PRINT_FEAT(FdpExcptnOnly, "x87 FDP updated only on exceptions");
    PRINT_FEAT(Smep, "Supervisor Mode Execution Prevention");
    PRINT_FEAT(Bmi2, "Bit Manipulation Instructions 2");
    PRINT_FEAT(Erms, "Enhanced REP MOVSB/STOSB");
    PRINT_FEAT(Invpcid, "INVPCID instruction");
    PRINT_FEAT(Rtm, "Restricted Transactional Memory");
    PRINT_FEAT(RdtMPqm, "Platform QoS Monitoring");
    PRINT_FEAT(FcsFdsDeprecation, "FCS/FDS Deprecation");
    PRINT_FEAT(Mpx, "Memory Protection Extensions");
    PRINT_FEAT(RdtAPqe, "Platform QoS Enforcement");
    PRINT_FEAT(Avx512F, "AVX-512 Foundation");
    PRINT_FEAT(Avx512Dq, "AVX-512 Doubleword/Quadword");
    PRINT_FEAT(Rdseed, "RDSEED");
    PRINT_FEAT(Adx, "Multi-precision Add-Carry Instructions");
    PRINT_FEAT(Smap, "Supervisor Mode Access Prevention");
    PRINT_FEAT(Avx512Ifma, "AVX-512 Integer Fused Multiply-Add");
    PRINT_FEAT(Pmcommit, "PCOMMIT instruction");
    PRINT_FEAT(Clflushopt, "CLFLUSHOPT");
    PRINT_FEAT(Clwb, "Cache Line Write Back");
    PRINT_FEAT(Pt, "Intel Processor Trace");
    PRINT_FEAT(Avx512Pf, "AVX-512 Prefetch");
    PRINT_FEAT(Avx512Er, "AVX-512 Exponential/Reciprocal");
    PRINT_FEAT(Avx512Cd, "AVX-512 Conflict Detection");
    PRINT_FEAT(Sha, "SHA Instructions");
    PRINT_FEAT(Avx512Bw, "AVX-512 Byte/Word");
    PRINT_FEAT(Avx512Vl, "AVX-512 Vector Length");
    PRINT_FEAT(Prefetchwt1, "PREFETCHWT1");
    PRINT_FEAT(Avx512Vbmi, "AVX-512 Vector Byte Manipulation");
    PRINT_FEAT(Umip, "User-Mode Instruction Prevention");
    PRINT_FEAT(Pku, "Protection Keys for User Pages");
    PRINT_FEAT(Ospke, "OS Protection Keys Enabled");
    PRINT_FEAT(Waitpkg, "WAITPKG instructions");
    PRINT_FEAT(Avx512Vbmi2, "AVX-512 VBMI2");
    PRINT_FEAT(CetSs, "Control-flow Enforcement Shadow Stack");
    PRINT_FEAT(Gfni, "Galois Field Instructions");
    PRINT_FEAT(Vaes, "Vector AES");
    PRINT_FEAT(Vpclmulqdq, "Vector Carry-less Multiply");
    PRINT_FEAT(Avx512Vnni, "AVX-512 Vector Neural Network Instructions");
    PRINT_FEAT(Avx512Bitalg, "AVX-512 Bit Algorithms");
    PRINT_FEAT(TmeEn, "Total Memory Encryption");
    PRINT_FEAT(Avx512Vpopcntdq, "AVX-512 Vector Population Count");
    PRINT_FEAT(Fzm, "Fast Zero-length MOVSB");
    PRINT_FEAT(La57, "57-bit Linear Addresses");
    PRINT_FEAT(Rdpid, "RDPID instruction");
    PRINT_FEAT(Kl, "Key Locker");
    PRINT_FEAT(BusLockDetect, "Bus Lock Detection");
    PRINT_FEAT(Cldemote, "CLDEMOTE");
    PRINT_FEAT(Mprr, "Memory Protection Range Registers");
    PRINT_FEAT(Movdiri, "MOVDIRI");
    PRINT_FEAT(Movdir64b, "MOVDIR64B");
    PRINT_FEAT(Enqcmd, "ENQCMD");
    PRINT_FEAT(SgxLc, "SGX Launch Configuration");
    PRINT_FEAT(Pks, "Protection Keys for Supervisor");
    PRINT_FEAT(SgxTerm, "SGX Trusted EREMOVE");
    PRINT_FEAT(SgxKeys, "SGX Attestation Keys");
    PRINT_FEAT(Avx512_4Vnniw, "AVX-512 4VNNIW");
    PRINT_FEAT(Avx512_4Fmaps, "AVX-512 4FMAPS");
    PRINT_FEAT(Fsrm, "Fast Short REP MOV");
    PRINT_FEAT(Uintr, "User Interrupts");
    PRINT_FEAT(Avx512Vp2intersect, "AVX-512 VP2INTERSECT");
    PRINT_FEAT(SrbdsCtrl, "SRBDS Mitigation");
    PRINT_FEAT(MdClear, "MD_CLEAR");
    PRINT_FEAT(RtmAlwaysAbort, "RTM Always Aborts");
    PRINT_FEAT(RtmForceAbort, "RTM Force Abort");
    PRINT_FEAT(Serialize, "SERIALIZE");
    PRINT_FEAT(Hybrid, "Hybrid Processor");
    PRINT_FEAT(Tsxldtrk, "TSX Load Tracking");
    PRINT_FEAT(Pconfig, "PCONFIG");
    PRINT_FEAT(Lbr, "Architectural Last Branch Records");
    PRINT_FEAT(CetIbt, "Control-flow Enforcement Indirect Branch Tracking");
    PRINT_FEAT(AmxBf16, "AMX BF16");
    PRINT_FEAT(Avx512Fp16, "AVX-512 FP16");
    PRINT_FEAT(AmxTile, "AMX Tile");
    PRINT_FEAT(AmxInt8, "AMX INT8");
    PRINT_FEAT(SpecCtrl, "Speculation Control");
    PRINT_FEAT(Stibp, "Single Thread Indirect Branch Predictors");
    PRINT_FEAT(L1dFlush, "L1 Data Cache Flush");
    PRINT_FEAT(ArchCapabilities, "Architectural Capabilities MSR");
    PRINT_FEAT(CoreCapabilities, "Core Capabilities MSR");
    PRINT_FEAT(Ssbd, "Speculative Store Bypass Disable");
    PRINT_FEAT(Sha512, "SHA-512 Instructions");
    PRINT_FEAT(Sm3, "SM3 Instructions");
    PRINT_FEAT(Sm4, "SM4 Instructions");
    PRINT_FEAT(RaoInt, "Remote Atomic Operations");
    PRINT_FEAT(AmxVnni, "AMX VNNI");
    PRINT_FEAT(Avx512Bf16, "AVX-512 BF16");
    PRINT_FEAT(Lass, "Linear Address Space Separation");
    PRINT_FEAT(Cmpccxadd, "CMPccXADD");
    PRINT_FEAT(Archperfmonext, "Architectural Performance Monitor Extensions");
    PRINT_FEAT(Dedup, "Memory Deduplication");
    PRINT_FEAT(Fzrm, "Fast Zero REP MOV");
    PRINT_FEAT(Fsrs, "Fast Short REP STOS");
    PRINT_FEAT(Rsrcs, "Return Stack Controls");
    PRINT_FEAT(Fred, "Flexible Return and Event Delivery");
    PRINT_FEAT(Lkgs, "Load Key from GS");
    PRINT_FEAT(Wrmsrns, "Non-serializing WRMSR");
    PRINT_FEAT(NmiSrc, "NMI Source Reporting");
    PRINT_FEAT(AmxFp16, "AMX FP16");
    PRINT_FEAT(Hreset, "History Reset");
    PRINT_FEAT(AvxIfma, "AVX Integer Fused Multiply-Add");
    PRINT_FEAT(Lam, "Linear Address Masking");
    PRINT_FEAT(Msrlist, "MSR List");
    PRINT_FEAT(InvdDisablePostBiosDone, "INVD Disable after BIOS");
    PRINT_FEAT(Movrs, "MOVRS");
    PRINT_FEAT(Ppin, "Protected Processor Inventory Number");
    PRINT_FEAT(Pbndkb, "PBNDKB");
    PRINT_FEAT(CpuidMaxvalLimRmv, "CPUID MaxVal Limit Removed");
    PRINT_FEAT(Mpsadbw512, "AVX-512 MPSADBW");
    PRINT_FEAT(Avx512RaoFp, "AVX-512 Remote Atomic FP");
    PRINT_FEAT(RdtMAsym, "Asymmetric QoS Monitoring");
    PRINT_FEAT(RdtAAsym, "Asymmetric QoS Enforcement");
    PRINT_FEAT(ReducedIsa, "Reduced ISA");
    PRINT_FEAT(Sipi64, "64-bit SIPI");
    PRINT_FEAT(MsrImm, "Immediate MSR Access");
    PRINT_FEAT(Ace, "Authenticated Code Execution");
    PRINT_FEAT(Avx512VnniFp16, "AVX-512 VNNI FP16");
    PRINT_FEAT(Avx512VnniInt8, "AVX-512 VNNI INT8");
    PRINT_FEAT(Avx512NeConvert, "AVX-512 Neural Convert");
    PRINT_FEAT(AvxVnniInt8, "AVX VNNI INT8");
    PRINT_FEAT(AvxNeConvert, "AVX Neural Convert");
    PRINT_FEAT(AmxComplex, "AMX Complex");
    PRINT_FEAT(AvxVnniInt16, "AVX VNNI INT16");
    PRINT_FEAT(Avx512VnniInt16, "AVX-512 VNNI INT16");
    PRINT_FEAT(Utmr, "User Timer");
    PRINT_FEAT(Prefetchi, "PREFETCHI");
    PRINT_FEAT(UserMsr, "User-mode MSRs");
    PRINT_FEAT(Avx512Bf16Ne, "AVX-512 BF16 Neural Extensions");
    PRINT_FEAT(UiretUifFromRflags, "UIRET Restores UIF");
    PRINT_FEAT(CetSss, "CET Supervisor Shadow Stack");
    PRINT_FEAT(Avx10, "AVX10");
    PRINT_FEAT(ApxF, "Advanced Performance Extensions");
    PRINT_FEAT(SecTeeAttestation, "Secure TEE Attestation");
    PRINT_FEAT(Mwait, "MWAIT");
    PRINT_FEAT(Slsm, "Supervisor Linear Speculation Mitigation");
    PRINT_FEAT(Psfd, "Predictive Store Forwarding Disable");
    PRINT_FEAT(IpredCtrl, "Indirect Prediction Control");
    PRINT_FEAT(RrsbaCtrl, "RRSBA Control");
    PRINT_FEAT(DdpuU, "Data Dependent Prefetcher Update");
    PRINT_FEAT(BhiCtrl, "Branch History Injection Control");
    PRINT_FEAT(McdtNo, "No MXCSR Configuration Dependent Timing");
    PRINT_FEAT(UcLockDisable, "UC Lock Disable");
    PRINT_FEAT(MonitorMitgNo, "No MONITOR Mitigation Required");
    printk("\r\n");
}

static VOID DumpProc(_PCpuidProcInfo Proc) {
    printk("\r\nProcessor info\r\n");
    printk("  Stepping:          0x%x\r\n", Proc->Stepping);
    printk("  Model:             0x%x\r\n", Proc->Model);
    printk("  Family:            0x%x\r\n", Proc->Family);
    printk("  Processor Type:    0x%x\r\n", Proc->ProcType);
    printk("  Ext Model:         0x%x\r\n", Proc->ExtModel);
    printk("  Ext Family:        0x%x\r\n", Proc->ExtFamily);
    printk("  Display Model:     0x%x\r\n", Proc->DisplayModel);
    printk("  Display Family:    0x%x\r\n", Proc->DisplayFamily);
    printk("  Brand Index:       0x%x\r\n", Proc->BrandIndex);
    printk("  Cache Line Size:   0x%xB\r\n", Proc->ClflushLineSize);
    printk("  Max Logical IDs:   0x%x\r\n", Proc->MaxLogicalIds);
    printk("  Initial APIC ID:   0x%x\r\n\r\n", Proc->InitialApicId);
}

/* ==========================================================================
 *                                                                          *
 * CpuidDump()                                                              *
 *                                                                          *
 * Dump decoded CPUID info                                                  *
 *                                                                          *
 ========================================================================== */
VOID CpuidDump(VOID) {
    /*
     * VGA text mode renders \t as ⚬, so I use spaces
     */
    printk("\r\n=== CPU ===\r\n\r\n");
    printk("Vendor:            %s\r\n", Info.Vendor);
    printk("Brand:             %s\r\n", Info.Brand);
    if (Info.Features.Psn) {
        printk("PSN:               %08x%08x%08x\r\n",
               Info.Psn[0], Info.Psn[1], Info.Psn[2]);
    }
    DumpProc(&Info.Proc);
    DumpFeat(&Info.Features);
    DumpExtFeat(&Info.FeaturesExt);
    DumpFeat7(&Info.Feat7);
}

_PCpuInfo CpuidGet(VOID) {
    return &Info;
}