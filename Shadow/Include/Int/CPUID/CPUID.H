/*
	* Shadow/Include/Int/CPUID/CPUID.H - CPU identification
	* Author:   amity
	* Date:     Tue Sep 29 14:54:23 2026
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
#ifndef __INT_CPUID_CPUID_H__
#define __INT_CPUID_CPUID_H__

// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_IOTEL         "GenuineIotel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "
 
// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

#define CPUID_RAW_MAX 128
/* --- Includes ---*/
/* --- Typedefs - Structs - Enums ---*/
typedef struct {
    UINT32 Leaf;
    UINT32 Subleaf;

    UINT32 Eax;
    UINT32 Ebx;
    UINT32 Ecx;
    UINT32 Edx;
} _CpuidRaw, *_PCpuidRaw;

typedef struct {
    _CpuidRaw Entries[CPUID_RAW_MAX];
    UINT32 Count;
} _CpuidRawDb, *_PCpuidRawDb;

// Leaf 1 feature flags, ECX register.
typedef enum {
    CPUID_FEAT_ECX_SSE3               = 1U << 0,
    CPUID_FEAT_ECX_PCLMUL             = 1U << 1,
    CPUID_FEAT_ECX_DTES64             = 1U << 2,
    CPUID_FEAT_ECX_MONITOR            = 1U << 3,
    CPUID_FEAT_ECX_DS_CPL             = 1U << 4,
    CPUID_FEAT_ECX_VMX                = 1U << 5,
    CPUID_FEAT_ECX_SMX                = 1U << 6,
    CPUID_FEAT_ECX_EST                = 1U << 7,
    CPUID_FEAT_ECX_TM2                = 1U << 8,
    CPUID_FEAT_ECX_SSSE3              = 1U << 9,
    CPUID_FEAT_ECX_CID                = 1U << 10,
    CPUID_FEAT_ECX_SDBG               = 1U << 11,
    CPUID_FEAT_ECX_FMA                = 1U << 12,
    CPUID_FEAT_ECX_CX16               = 1U << 13,
    CPUID_FEAT_ECX_XTPR               = 1U << 14,
    CPUID_FEAT_ECX_PDCM               = 1U << 15,
    CPUID_FEAT_ECX_PCID               = 1U << 17,
    CPUID_FEAT_ECX_DCA                = 1U << 18,
    CPUID_FEAT_ECX_SSE4_1             = 1U << 19,
    CPUID_FEAT_ECX_SSE4_2             = 1U << 20,
    CPUID_FEAT_ECX_X2APIC             = 1U << 21,
    CPUID_FEAT_ECX_MOVBE              = 1U << 22,
    CPUID_FEAT_ECX_POPCNT             = 1U << 23,
    CPUID_FEAT_ECX_TSC_DEADLINE       = 1U << 24,
    CPUID_FEAT_ECX_AES                = 1U << 25,
    CPUID_FEAT_ECX_XSAVE              = 1U << 26,
    CPUID_FEAT_ECX_OSXSAVE            = 1U << 27,
    CPUID_FEAT_ECX_AVX                = 1U << 28,
    CPUID_FEAT_ECX_F16C               = 1U << 29,
    CPUID_FEAT_ECX_RDRAND             = 1U << 30,
    CPUID_FEAT_ECX_HYPERVISOR         = 1U << 31
} _CpuidFeatEcx, *_PCpuidFeatEcx;

// Leaf 1 feature flags, EDX register.
typedef enum {
    CPUID_FEAT_EDX_FPU                = 1U << 0,
    CPUID_FEAT_EDX_VME                = 1U << 1,
    CPUID_FEAT_EDX_DE                 = 1U << 2,
    CPUID_FEAT_EDX_PSE                = 1U << 3,
    CPUID_FEAT_EDX_TSC                = 1U << 4,
    CPUID_FEAT_EDX_MSR                = 1U << 5,
    CPUID_FEAT_EDX_PAE                = 1U << 6,
    CPUID_FEAT_EDX_MCE                = 1U << 7,
    CPUID_FEAT_EDX_CX8                = 1U << 8,
    CPUID_FEAT_EDX_APIC               = 1U << 9,
    CPUID_FEAT_EDX_SEP                = 1U << 11,
    CPUID_FEAT_EDX_MTRR               = 1U << 12,
    CPUID_FEAT_EDX_PGE                = 1U << 13,
    CPUID_FEAT_EDX_MCA                = 1U << 14,
    CPUID_FEAT_EDX_CMOV               = 1U << 15,
    CPUID_FEAT_EDX_PAT                = 1U << 16,
    CPUID_FEAT_EDX_PSE36              = 1U << 17,
    CPUID_FEAT_EDX_PSN                = 1U << 18,
    CPUID_FEAT_EDX_CLFLUSH            = 1U << 19,
    CPUID_FEAT_EDX_DS                 = 1U << 21,
    CPUID_FEAT_EDX_ACPI               = 1U << 22,
    CPUID_FEAT_EDX_MMX                = 1U << 23,
    CPUID_FEAT_EDX_FXSR               = 1U << 24,
    CPUID_FEAT_EDX_SSE                = 1U << 25,
    CPUID_FEAT_EDX_SSE2               = 1U << 26,
    CPUID_FEAT_EDX_SS                 = 1U << 27,
    CPUID_FEAT_EDX_HTT                = 1U << 28,
    CPUID_FEAT_EDX_TM                 = 1U << 29,
    CPUID_FEAT_EDX_IA64               = 1U << 30,
    CPUID_FEAT_EDX_PBE                = 1U << 31
} _CpuidFeatEdx, *_PCpuidFeatEdx;

// Leaf 7, sub-leaf 0, EBX feature flags.
typedef enum {
    CPUID_FEAT7_0_EBX_FSGSBASE        = 1U << 0,
    CPUID_FEAT7_0_EBX_TSC_ADJUST      = 1U << 1,
    CPUID_FEAT7_0_EBX_SGX             = 1U << 2,
    CPUID_FEAT7_0_EBX_BMI1            = 1U << 3,
    CPUID_FEAT7_0_EBX_HLE             = 1U << 4,
    CPUID_FEAT7_0_EBX_AVX2            = 1U << 5,
    CPUID_FEAT7_0_EBX_FDP_EXCPTN_ONLY = 1U << 6,
    CPUID_FEAT7_0_EBX_SMEP            = 1U << 7,
    CPUID_FEAT7_0_EBX_BMI2            = 1U << 8,
    CPUID_FEAT7_0_EBX_ERMS            = 1U << 9,
    CPUID_FEAT7_0_EBX_INVPCID         = 1U << 10,
    CPUID_FEAT7_0_EBX_RTM             = 1U << 11,
    CPUID_FEAT7_0_EBX_RDT_M           = 1U << 12,
    CPUID_FEAT7_0_EBX_DEP_FPU_CS_DS   = 1U << 13,
    CPUID_FEAT7_0_EBX_MPX             = 1U << 14,
    CPUID_FEAT7_0_EBX_RDT_A           = 1U << 15,
    CPUID_FEAT7_0_EBX_AVX512F         = 1U << 16,
    CPUID_FEAT7_0_EBX_AVX512DQ        = 1U << 17,
    CPUID_FEAT7_0_EBX_RDSEED          = 1U << 18,
    CPUID_FEAT7_0_EBX_ADX             = 1U << 19,
    CPUID_FEAT7_0_EBX_SMAP            = 1U << 20,
    CPUID_FEAT7_0_EBX_AVX512IFMA      = 1U << 21,
    CPUID_FEAT7_0_EBX_PCOMMIT         = 1U << 22, // Deprecated
    CPUID_FEAT7_0_EBX_CLFLUSHOPT      = 1U << 23,
    CPUID_FEAT7_0_EBX_CLWB            = 1U << 24,
    CPUID_FEAT7_0_EBX_INTEL_PT        = 1U << 25,
    CPUID_FEAT7_0_EBX_AVX512PF        = 1U << 26,
    CPUID_FEAT7_0_EBX_AVX512ER        = 1U << 27,
    CPUID_FEAT7_0_EBX_AVX512CD        = 1U << 28,
    CPUID_FEAT7_0_EBX_SHA             = 1U << 29,
    CPUID_FEAT7_0_EBX_AVX512BW        = 1U << 30,
    CPUID_FEAT7_0_EBX_AVX512VL        = 1U << 31
} _CpuidFeat70Ebx, *_PCpuidFeat70Ebx;

// Leaf 7, sub-leaf 0, ECX feature flags.
typedef enum {
    CPUID_FEAT7_0_ECX_PREFETCHWT1     = 1U << 0,
    CPUID_FEAT7_0_ECX_AVX512_VBMI     = 1U << 1,
    CPUID_FEAT7_0_ECX_UMIP            = 1U << 2,
    CPUID_FEAT7_0_ECX_PKU             = 1U << 3,
    CPUID_FEAT7_0_ECX_OSPKE           = 1U << 4,
    CPUID_FEAT7_0_ECX_WAITPKG         = 1U << 5,
    CPUID_FEAT7_0_ECX_AVX512_VBMI2    = 1U << 6,
    CPUID_FEAT7_0_ECX_CET_SS          = 1U << 7,
    CPUID_FEAT7_0_ECX_GFNI            = 1U << 8,
    CPUID_FEAT7_0_ECX_VAES            = 1U << 9,
    CPUID_FEAT7_0_ECX_VPCLMULQDQ      = 1U << 10,
    CPUID_FEAT7_0_ECX_AVX512_VNNI     = 1U << 11,
    CPUID_FEAT7_0_ECX_AVX512_BITALG   = 1U << 12,
    CPUID_FEAT7_0_ECX_TME             = 1U << 13,
    CPUID_FEAT7_0_ECX_AVX512_VPOPCNTDQ= 1U << 14,
    CPUID_FEAT7_0_ECX_LA57            = 1U << 16,
    CPUID_FEAT7_0_ECX_RDPID           = 1U << 22,
    CPUID_FEAT7_0_ECX_KL              = 1U << 23,
    CPUID_FEAT7_0_ECX_CLDEMOTE        = 1U << 25,
    CPUID_FEAT7_0_ECX_MOVDIRI         = 1U << 27,
    CPUID_FEAT7_0_ECX_MOVDIR64B       = 1U << 28,
    CPUID_FEAT7_0_ECX_ENQCMD          = 1U << 29,
    CPUID_FEAT7_0_ECX_SGX_LC          = 1U << 30,
    CPUID_FEAT7_0_ECX_PKS             = 1U << 31
} _CpuidFeat70Ecx, *_PCpuidFeat70Ecx;

// Leaf 7, sub-leaf 0, EDX feature flags.
typedef enum {
    CPUID_FEAT7_0_EDX_AVX512_4VNNIW   = 1U << 2,
    CPUID_FEAT7_0_EDX_AVX512_4FMAPS   = 1U << 3,
    CPUID_FEAT7_0_EDX_FSRM            = 1U << 4,
    CPUID_FEAT7_0_EDX_AVX512_VP2INTERSECT = 1U << 8,
    CPUID_FEAT7_0_EDX_SRBDS_CTRL      = 1U << 9,
    CPUID_FEAT7_0_EDX_MD_CLEAR        = 1U << 10,
    CPUID_FEAT7_0_EDX_RTM_ALWAYS_ABORT= 1U << 11,
    CPUID_FEAT7_0_EDX_RTM_FORCE_ABORT = 1U << 12,
    CPUID_FEAT7_0_EDX_SERIALIZE       = 1U << 14,
    CPUID_FEAT7_0_EDX_TSXLDTRK        = 1U << 16,
    CPUID_FEAT7_0_EDX_PCONFIG         = 1U << 18,
    CPUID_FEAT7_0_EDX_ARCH_LBR        = 1U << 19,
    CPUID_FEAT7_0_EDX_CET_IBT         = 1U << 20,
    CPUID_FEAT7_0_EDX_AMX_BF16        = 1U << 22,
    CPUID_FEAT7_0_EDX_AVX512_FP16     = 1U << 23,
    CPUID_FEAT7_0_EDX_AMX_TILE        = 1U << 24,
    CPUID_FEAT7_0_EDX_AMX_INT8        = 1U << 25,
    CPUID_FEAT7_0_EDX_IBPB            = 1U << 26,
    CPUID_FEAT7_0_EDX_STIBP           = 1U << 27,
    CPUID_FEAT7_0_EDX_L1D_FLUSH       = 1U << 28,
    CPUID_FEAT7_0_EDX_ARCH_CAPABILITIES= 1U << 29,
    CPUID_FEAT7_0_EDX_CORE_CAPABILITIES = 1U << 30,
    CPUID_FEAT7_0_EDX_SSBD            = 1U << 31
} _CpuidFeat70Edx, *_PCpuidFeat70Edx;

// Leaf 0x80000001, ECX extended feature flags.
typedef enum {
    CPUID_FEAT_EXT_ECX_LAHF_LM        = 1U << 0,
    CPUID_FEAT_EXT_ECX_CMP_LEGACY     = 1U << 1,
    CPUID_FEAT_EXT_ECX_SVM            = 1U << 2,
    CPUID_FEAT_EXT_ECX_EXT_APIC_SPACE = 1U << 3,
    CPUID_FEAT_EXT_ECX_ALT_MOV_CR8    = 1U << 4,
    CPUID_FEAT_EXT_ECX_ABM            = 1U << 5,
    CPUID_FEAT_EXT_ECX_SSE4A          = 1U << 6,
    CPUID_FEAT_EXT_ECX_MISALIGN_SSE   = 1U << 7,
    CPUID_FEAT_EXT_ECX_3DNOW_PREFETCH = 1U << 8,
    CPUID_FEAT_EXT_ECX_OSVW           = 1U << 9,
    CPUID_FEAT_EXT_ECX_IBS            = 1U << 10,
    CPUID_FEAT_EXT_ECX_XOP            = 1U << 11,
    CPUID_FEAT_EXT_ECX_SKINIT         = 1U << 12,
    CPUID_FEAT_EXT_ECX_WDT            = 1U << 13,
    CPUID_FEAT_EXT_ECX_LWP            = 1U << 15,
    CPUID_FEAT_EXT_ECX_FMA4           = 1U << 16,
    CPUID_FEAT_EXT_ECX_TCE            = 1U << 17,
    CPUID_FEAT_EXT_ECX_NODEID_MSR     = 1U << 19,
    CPUID_FEAT_EXT_ECX_TBM            = 1U << 21,
    CPUID_FEAT_EXT_ECX_TOPOEXT        = 1U << 22,
    CPUID_FEAT_EXT_ECX_PERFCTR_CORE   = 1U << 23,
    CPUID_FEAT_EXT_ECX_PERFCTR_NB     = 1U << 24,
    CPUID_FEAT_EXT_ECX_DBX            = 1U << 26,
    CPUID_FEAT_EXT_ECX_PERFTSC        = 1U << 27,
    CPUID_FEAT_EXT_ECX_PCX_L2I        = 1U << 28,
    CPUID_FEAT_EXT_ECX_MONITORX       = 1U << 29,
    CPUID_FEAT_EXT_ECX_ADDR_MASK_EXT  = 1U << 30
} _CpuidFeatExtEcx, *_PCpuidFeatExtEcx;

// Leaf 0x80000001, EDX extended feature flags.
typedef enum {
    CPUID_FEAT_EXT_EDX_FPU            = 1U << 0,
    CPUID_FEAT_EXT_EDX_VME            = 1U << 1,
    CPUID_FEAT_EXT_EDX_DE             = 1U << 2,
    CPUID_FEAT_EXT_EDX_PSE            = 1U << 3,
    CPUID_FEAT_EXT_EDX_TSC            = 1U << 4,
    CPUID_FEAT_EXT_EDX_MSR            = 1U << 5,
    CPUID_FEAT_EXT_EDX_PAE            = 1U << 6,
    CPUID_FEAT_EXT_EDX_MCE            = 1U << 7,
    CPUID_FEAT_EXT_EDX_CX8            = 1U << 8,
    CPUID_FEAT_EXT_EDX_APIC           = 1U << 9,
    CPUID_FEAT_EXT_EDX_SYSCALL        = 1U << 11,
    CPUID_FEAT_EXT_EDX_MTRR           = 1U << 12,
    CPUID_FEAT_EXT_EDX_PGE            = 1U << 13,
    CPUID_FEAT_EXT_EDX_MCA            = 1U << 14,
    CPUID_FEAT_EXT_EDX_CMOV           = 1U << 15,
    CPUID_FEAT_EXT_EDX_PAT            = 1U << 16,
    CPUID_FEAT_EXT_EDX_PSE36          = 1U << 17,
    CPUID_FEAT_EXT_EDX_NX             = 1U << 20,
    CPUID_FEAT_EXT_EDX_MMXEXT         = 1U << 22,
    CPUID_FEAT_EXT_EDX_MMX            = 1U << 23,
    CPUID_FEAT_EXT_EDX_FXSR           = 1U << 24,
    CPUID_FEAT_EXT_EDX_FFXSR          = 1U << 25,
    CPUID_FEAT_EXT_EDX_PAGE1GB        = 1U << 26,
    CPUID_FEAT_EXT_EDX_RDTSCP         = 1U << 27,
    CPUID_FEAT_EXT_EDX_LM             = 1U << 29,
    CPUID_FEAT_EXT_EDX_3DNOWEXT       = 1U << 30,
    CPUID_FEAT_EXT_EDX_3DNOW          = 1U << 31
} _CpuidFeatExtEdx, *_PCpuidFeatExtEdx;

// Processor info from leaf 1.
typedef struct {
    UINT8  Stepping;
    UINT8  Model;
    UINT8  Family;
    UINT8  ProcType;
    UINT8  ExtModel;
    UINT8  ExtFamily;
    UINT8  DisplayModel;         // Computed per Intel/AMD rules
    UINT8  DisplayFamily;        // Computed per Intel/AMD rules
    UINT8  BrandIndex;
    UINT8  ClflushLineSize;
    UINT8  MaxLogicalIds;
    UINT32 InitialApicId;
} _CpuidProcInfo, *_PCpuidProcInfo;

// Cache topology from leaf 4.
typedef struct {
    // EAX
    UINT8  CacheType;            // 0=Null, 1=Data, 2=Inst, 3=Unified
    UINT8  CacheLevel;
    UINT8  SelfInitializing;
    UINT8  FullyAssociative;
    UINT32 MaxThreadsSharing;
    UINT32 MaxCoresSharing;
    // EBX
    UINT32 LineSize; // -1
    UINT32 LinePartitions; // -1
    UINT32 Ways; // =1
    // EDX
    UINT32 Sets;
    UINT8  Wbinvd;
    UINT8  Inclusive;
    UINT8  ComplexIndexing;
} _CpuidCacheInfo, *_PCpuidCacheInfo;

// MONITOR/MWAIT from leaf 5.
typedef struct {
    // EAX
    UINT32 SmallestLine;
    // EBX
    UINT32 LargestLine;
    // ECX
    UINT8  Extensions;
    UINT8  InterruptsAsBreak;
    UINT8  Monitorless;
    // EDX
    UINT32 C0Substates;
    UINT32 C1Substates;
    UINT32 CSubstates;
    UINT32 C3Substates;
    UINT32 C4Substates;
    UINT32 C5Substates;
    UINT32 C6Substates;
    UINT32 C7Substates;
} _CpuidMwaitInfo, *_PCpuidMwaitInfo;

// Thermal and power from leaf 6.
typedef struct {
    // EAX
    UINT8  DigitalTempSensor;
    UINT8  TurboBoost;
    UINT8  Arat;
    // reserved
    UINT8  Pln;
    UINT8  Ecmd;
    UINT8  Ptm;
    UINT8  Hwp;
    UINT8  HwpN;
    UINT8  HwpAw;
    UINT8  HwpEpp;
    UINT8  HwpPlr;
    // reserved
    UINT8  Hdc;
    UINT8  TurboBoostMax;
    UINT8  HwpCap;
    UINT8  HwpPeci;
    UINT8  HwpFlex;
    UINT8  HwpRfa;
    UINT8  HwpFeedback;
    UINT8  HwpRii;
    // reserved
    UINT8  HwpControl;
    UINT8  ThreadDirector;
    // EBX
    UINT32 InterruptThresholds;
    // ECX
    UINT8  EffectiveFrequencyInterface;
    UINT8  Acnt2;
    // reserved
    UINT8  EnergyPerfBias;
    // reserved
    UINT32 ThreadDirectorClasses;
    // EDX
    UINT8  PerformanceCapabilityReporting;
    UINT8  EfficiencyCapabilityReporting;
    UINT32 HardwareFeedbackInterfaceSize; // * 4 kiB - 1
    UINT32 LogicalRowIdx;
} _CpuidThermalInfo, *_PCpuidThermalInfo;

// Extended features from leaf 7
typedef struct {
    /* --- Sub Leaf 0 --- */
    // EBX
    UINT8 Fsgsbase;
    UINT8 TscAdjust;
    UINT8 Sgx;
    UINT8 Bmi1;
    UINT8 Hle;
    UINT8 Avx2;
    UINT8 FdpExcptnOnly;
    UINT8 Smep;
    UINT8 Bmi2;
    UINT8 Erms;
    UINT8 Invpcid;
    UINT8 Rtm;
    UINT8 RdtMPqm;
    UINT8 FcsFdsDeprecation;
    UINT8 Mpx;
    UINT8 RdtAPqe;
    UINT8 Avx512F;
    UINT8 Avx512Dq;
    UINT8 Rdseed;
    UINT8 Adx;
    UINT8 Smap;
    UINT8 Avx512Ifma;
    UINT8 Pmcommit;
    UINT8 Clflushopt;
    UINT8 Clwb;
    UINT8 Pt;
    UINT8 Avx512Pf;
    UINT8 Avx512Er;
    UINT8 Avx512Cd;
    UINT8 Sha;
    UINT8 Avx512Bw;
    UINT8 Avx512Vl;
    // ECX
    UINT8 Prefetchwt1;
    UINT8 Avx512Vbmi;
    UINT8 Umip;
    UINT8 Pku;
    UINT8 Ospke;
    UINT8 Waitpkg;
    UINT8 Avx512Vbmi2;
    UINT8 CetSs;
    UINT8 Gfni;
    UINT8 Vaes;
    UINT8 Vpclmulqdq;
    UINT8 Avx512Vnni;
    UINT8 Avx512Bitalg;
    UINT8 TmeEn;
    UINT8 Avx512Vpopcntdq;
    UINT8 Fzm;
    UINT8 La57;
    UINT32 Mawau;
    UINT8 Rdpid;
    UINT8 Kl;
    UINT8 BusLockDetect;
    UINT8 Cldemote;
    UINT8 Mprr;
    UINT8 Movdiri;
    UINT8 Movdir64b;
    UINT8 Enqcmd;
    UINT8 SgxLc;
    UINT8 Pks;
    // EDX
    UINT8 SgxTerm;
    UINT8 SgxKeys;
    UINT8 Avx512_4Vnniw;
    UINT8 Avx512_4Fmaps;
    UINT8 Fsrm;
    UINT8 Uintr;
    // reserved
    // reserved
    UINT8 Avx512Vp2intersect;
    UINT8 SrbdsCtrl;
    UINT8 MdClear;
    UINT8 RtmAlwaysAbort;
    // reserved
    UINT8 RtmForceAbort;
    UINT8 Serialize;
    UINT8 Hybrid;
    UINT8 Tsxldtrk;
    // reserved
    UINT8 Pconfig;
    UINT8 Lbr;
    UINT8 CetIbt;
    // reserved
    UINT8 AmxBf16;
    UINT8 Avx512Fp16;
    UINT8 AmxTile;
    UINT8 AmxInt8;
    UINT8 SpecCtrl;
    UINT8 Stibp;
    UINT8 L1dFlush;
    UINT8 ArchCapabilities;
    UINT8 CoreCapabilities;
    UINT8 Ssbd;
    /* --- Sub Leaf 1 --- */
    // EAX
    UINT8 Sha512;
    UINT8 Sm3;
    UINT8 Sm4;
    UINT8 RaoInt;
    UINT8 AmxVnni;
    UINT8 Avx512Bf16;
    UINT8 Lass;
    UINT8 Cmpccxadd;
    UINT8 Archperfmonext;
    UINT8 Dedup;
    UINT8 Fzrm;
    UINT8 Fsrs;
    UINT8 Rsrcs;
    // reserved
    // reserved
    // reserved
    // reserved
    UINT8 Fred;
    UINT8 Lkgs;
    UINT8 Wrmsrns;
    UINT8 NmiSrc;
    UINT8 AmxFp16;
    UINT8 Hreset;
    UINT8 AvxIfma;
    // reserved
    // reserved
    UINT8 Lam;
    UINT8 Msrlist;
    // reserved
    // reserved
    UINT8 InvdDisablePostBiosDone;
    UINT8 Movrs;
    // EBX
    UINT8 Ppin;
    UINT8 Pbndkb;
    // reserved
    UINT8 CpuidMaxvalLimRmv;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    UINT8 Mpsadbw512;
    // reserved
    UINT8 Avx512RaoFp;
    // ECX
    UINT8 RdtMAsym;
    UINT8 RdtAAsym;
    UINT8 ReducedIsa;
    // reserved
    UINT8 Sipi64;
    UINT8 MsrImm;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    UINT8 Ace;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // EDX
    // reserved
    UINT8 Avx512VnniFp16;
    UINT8 Avx512VnniInt8;
    UINT8 Avx512NeConvert;
    UINT8 AvxVnniInt8;
    UINT8 AvxNeConvert;
    // reserved
    // reserved
    UINT8 AmxComplex;
    // reserved
    UINT8 AvxVnniInt16;
    UINT8 Avx512VnniInt16;
    // reserved
    UINT8 Utmr;
    UINT8 Prefetchi;
    UINT8 UserMsr;
    UINT8 Avx512Bf16Ne;
    UINT8 UiretUifFromRflags;
    UINT8 CetSss;
    UINT8 Avx10;
    // reserved
    UINT8 ApxF;
    UINT8 SecTeeAttestation;
    UINT8 Mwait;
    UINT8 Slsm;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    /* --- Sub Leaf 2 --- */
    // EDX
    UINT8 Psfd;
    UINT8 IpredCtrl;
    UINT8 RrsbaCtrl;
    UINT8 DdpuU;
    UINT8 BhiCtrl;
    UINT8 McdtNo;
    UINT8 UcLockDisable;
    UINT8 MonitorMitgNo;
} _CpuidFeat7, *_PCpuidFeat7;

// Address sizes from leaf 0x80000008.
typedef struct {
    // EBX
    UINT8 Clzero;
    UINT8 RetiredInstr;
    UINT8 XrstorFpErr;
    UINT8 Invlpgb;
    UINT8 Rdpru;
    UINT8 Xotext; // Playstation 5????
    UINT8 Mbe;
    // reserved
    UINT8 Mcommit;
    UINT8 Wbnoinvd;
    UINT8 LbrExtV1;
    // reserved
    UINT8 Ibpb;
    UINT8 WbinvdInt;
    UINT8 Ibrs;
    UINT8 Stibp;
    UINT8 IbrsAlwaysOn;
    UINT8 StibpAlwaysOn;
    UINT8 IbrsPreferred;
    UINT8 IbrsSameModeProtection;
    UINT8 NoEferLmsle;
    UINT8 InvlpgbNested;
    UINT8 LbrTsx;
    UINT8 Ppin;
    UINT8 Ssbd;
    UINT8 SsbdLegacy;
    UINT8 SsbdNo;
    UINT8 Cppc;
    UINT8 Psfd;
    UINT8 BtcNo;
    UINT8 IbpbRet;
    UINT8 BranchSampling;
    // EAX
    UINT32 NPhysicalAddrBits;
    UINT32 NLinearAddrBits;
    UINT32 GuestPhysicalAddrSize;
    // ECX
    UINT32 NPhysicalThreads; // -1
    UINT32 ApicIdSize;
    UINT32 PerfTimestepCounterSize;
    // EDX
    UINT32 InvlpgbMaxPages;
    UINT32 MaxEcxRdpru;
} _CpuidExtFunc2, *_PCpuidExtFunc2;

// SVM features from leaf 0x8000000A (AMD).
typedef struct {
    // EAX
    UINT32 SvmRevision;
    // EBX
    UINT32 Nasid;
    // EDX
    UINT8  NestedPaging;
    UINT8  LbrVirt;
    UINT8  SvmLock;
    UINT8  NripSave;
    UINT8  TscRateMsr;
    UINT8  VmcbClean;
    UINT8  FlushByAsid;
    UINT8  DecodeAssists;
    UINT8  PcmVirt;
    UINT8  PauseFilter;
    UINT8  Emp;
    UINT8  Pft;
    UINT8  Avic;
    UINT8  VVmsaveVmload;
    UINT8  Vgif;
    UINT8  Gmet;
    UINT8  X2avic;
    UINT8  Ssscheck;
    UINT8  SpecCtrl;
    UINT8  Rogpt;
    UINT8  HostMceOverride;
    UINT8  Tlbictl;
    UINT8  Vnmi;
    UINT8  Lbsvirt;
    UINT8  Extlvtoffsetfaultchg;
    UINT8  Vmcbaddrchkchg;
    UINT8  Blt;
    UINT8  Ihi;
    UINT8  Esi;
    // ECX
    UINT8  Gpcmef;
    UINT8  Pml;
    UINT8  X2avicExt;
} _CpuidSvmInfo, *_PCpuidSvmInfo;

typedef struct {
    // EDX
    UINT8 Fpu;
    UINT8 Vme;
    UINT8 De;
    UINT8 Pse;
    UINT8 Tsc;
    UINT8 Msr;
    UINT8 Pae;
    UINT8 Mce;
    UINT8 Cx8;
    UINT8 Apic;
    // reserved
    UINT8 Sep;
    UINT8 Mtrr;
    UINT8 Pge;
    UINT8 Mca;
    UINT8 Cmov;
    UINT8 Pat;
    UINT8 Pse36;
    UINT8 Psn;
    UINT8 Clfsh;
    UINT8 Nx;
    UINT8 Ds;
    UINT8 Acpi;
    UINT8 Mmx;
    UINT8 Fxsr;
    UINT8 Sse;
    UINT8 Sse2;
    UINT8 Ss;
    UINT8 Htt;
    UINT8 Tm;
    UINT8 Ia64;
    UINT8 Pbe;
    // ECX
    UINT8 Sse3;
    UINT8 Pclmulqdq;
    UINT8 Dtes64;
    UINT8 Monitor;
    UINT8 DsCpl;
    UINT8 Vmx;
    UINT8 Smx;
    UINT8 Est;
    UINT8 Tm2;
    UINT8 Ssse3;
    UINT8 CntxId;
    UINT8 Sdbg;
    UINT8 Fma;
    UINT8 Cx16;
    UINT8 Xtpr;
    UINT8 Pdcm;
    // reserved
    UINT8 Pcid;
    UINT8 Dca;
    UINT8 Sse41;
    UINT8 Sse42;
    UINT8 X2apic;
    UINT8 Movbe;
    UINT8 Popcnt;
    UINT8 TscDeadline;
    UINT8 AesNi;
    UINT8 Xsave;
    UINT8 Osxsave;
    UINT8 Avx;
    UINT8 F16c;
    UINT8 Rdrnd;
    UINT8 Hypervisor;
} _CpuidFeat, *_PCpuidFeat;

typedef struct {
    // EDX
    UINT8 Fpu;
    UINT8 Vme;
    UINT8 De;
    UINT8 Pse;
    UINT8 Tsc;
    UINT8 Msr;
    UINT8 Pae;
    UINT8 Mce;
    UINT8 Cx8;
    UINT8 Apic;
    UINT8 SyscallK6;
    UINT8 Syscall;
    UINT8 Mtrr;
    UINT8 Pge;
    UINT8 Mca;
    UINT8 Cmov;
    UINT8 Pat;
    UINT8 Pse36;
    UINT8 EccK7;
    UINT8 Ecc;
    UINT8 Nx;
    UINT8 Sem;
    UINT8 Mmxext;
    UINT8 Mmx;
    UINT8 Fxsr;
    UINT8 FxsrOpt;
    UINT8 Pdpe1gb;
    UINT8 Rdtscp;
    UINT8 Rex32K8;
    UINT8 Lm;
    UINT8 Tdnowext;
    UINT8 Tdnow;
    // ECX
    UINT8 LahfLm;
    UINT8 CmpLegacy;
    UINT8 Svm;
    UINT8 Extapic;
    UINT8 Cr8Legacy;
    UINT8 Abm;
    UINT8 Sse4a;
    UINT8 Misalignsse;
    UINT8 Tdnowprefetch;
    UINT8 Osvw;
    UINT8 Ibs;
    UINT8 Xop;
    UINT8 Skinit;
    UINT8 Wdt;
    UINT8 Tbm0;
    UINT8 Lwp;
    UINT8 Fma4;
    UINT8 Tce;
    UINT8 Cvt16;
    UINT8 NodeidMsr;
    // reserved
    UINT8 Tbm;
    UINT8 Topoext;
    UINT8 PerfctrCore;
    UINT8 PerfctrNb;
    UINT8 StreamPerfMon;
    UINT8 Dbx;
    UINT8 Perftsc;
    UINT8 PcxL2iL3;
    UINT8 Monitorx;
    UINT8 AddrMaskExt;
    // reserved
} _CpuidExtFeat, *_PCpuidExtFeat;

// Topology from leaf 0x0B (Intel extended topology).
typedef struct {
    UINT8  LevelType;            // 1=Thread, 2=Core, 3=Module, etc.
    UINT32 LogicalPerLevel;
    UINT32 LevelNumber;
    UINT32 X2apicId;
    UINT8  Valid;
} _CpuidTopology, *_PCpuidTopology;

// Frequency info from leaves 0x15 and 0x16.
typedef struct {
    UINT32 TscDenominator;
    UINT32 TscNumerator;
    UINT64 TscFreqHz;
    UINT32 BaseFreqMhz;         // Leaf 0x16 EAX
    UINT32 MaxFreqMhz;          // Leaf 0x16 EBX
    UINT32 BusFreqMhz;          // Leaf 0x16 ECX
} _CpuidFreqInfo, *_PCpuidFreqInfo;

// Main CPU info structure.
typedef struct {
    char     Vendor[13];
    char     Brand[49];

    UINT32 Psn[3];

    UINT32 MaxBasicLeaf;
    UINT32 MaxExtendedLeaf;

    _CpuidProcInfo   Proc;
    _CpuidFeat       Features;
    _CpuidExtFeat    FeaturesExt;
    _CpuidFeat7      Feat7;

    _CpuidMwaitInfo  Mwait;
    _CpuidThermalInfo Thermal;

    _CpuidExtFunc2   AddrSize;
    _CpuidFreqInfo   Freq;

    // --- Embedded sub-structures ---

    _CpuidCacheInfo  Caches[8];      // Leaf 4: usually 4-6 caches
    UINT32            CacheCount;
    _CpuidTopology    Topo[8];        // Leaf 0x0B: usually 2-3 levels
    UINT32            TopoCount;
    _CpuidSvmInfo     Svm;            // Leaf 0x8000000A

    UINT8             HasMwait;
    UINT8             HasThermal;
    UINT8             HasSvm;
    UINT8             HasTopology;
} _CpuInfo, *_PCpuInfo;

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif