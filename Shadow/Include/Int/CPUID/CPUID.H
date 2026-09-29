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
    UINT32 leaf;
    UINT32 subleaf;

    UINT32 eax;
    UINT32 ebx;
    UINT32 ecx;
    UINT32 edx;
} _CpuidRaw, *_PCpuidRaw;

typedef struct {
    _CpuidRaw entries[CPUID_RAW_MAX];
    UINT32 count;
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
    UINT8  Ext_family;
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
    // EDS
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
    UINT8  digital_temp_sensor;
    UINT8  turbo_boost;
    UINT8  arat;
    // reserved
    UINT8  pln;
    UINT8  ecmd;
    UINT8  ptm;
    UINT8  hwp;
    UINT8  hwp_n;
    UINT8  hwp_aw;
    UINT8  hwp_epp;
    UINT8  hwp_plr;
    // reserved
    UINT8  hdc;
    UINT8  turbo_boost_max;
    UINT8  hwp_cap;
    UINT8  hwp_peci;
    UINT8  hwp_flex;
    UINT8  hwp_rfa;
    UINT8  hwp_feedback;
    UINT8  hwp_rii;
    // reserved
    UINT8  hwp_control;
    UINT8  thread_director;
    // EBX
    UINT32 interrupt_thresholds;
    // ECX
    UINT8  effective_frequency_interface;
    UINT8  acnt2;
    // reserved
    UINT8  energy_perf_bias;
    //reserved
    UINT32 thread_director_classes;
    // EDX
    UINT8 performance_capability_reporting;
    UINT8 efficiency_capability_reporting;
    UINT32 hardware_feedback_interface_size; // * 4 kiB - 1
    UINT32 logical_row_idx;
} _CpuidThermalInfo, *_PCpuidThermalInfo;

// Extended features from leaf 7
typedef struct {
    /* --- Sub Leaf 0 --- */
    // EBX
    UINT8 fsgsbase;
    UINT8 tsc_adjust;
    UINT8 sgx;
    UINT8 bmi1;
    UINT8 hle;
    UINT8 avx2;
    UINT8 fdp_excptn_only;
    UINT8 smep;
    UINT8 bmi2;
    UINT8 erms;
    UINT8 invpcid;
    UINT8 rtm;
    UINT8 rdt_m_pqm;
    UINT8 fcs_fds_deprecation;
    UINT8 mpx;
    UINT8 rdt_a_pqe;
    UINT8 avx512_f;
    UINT8 avx512_dq;
    UINT8 rdseed;
    UINT8 adx;
    UINT8 smap;
    UINT8 avx512_ifma;
    UINT8 pmcommit;
    UINT8 clflushopt;
    UINT8 clwb;
    UINT8 pt;
    UINT8 avx512_pf;
    UINT8 avx512_er;;
    UINT8 avx512_cd;
    UINT8 sha;
    UINT8 avx512_bw;
    UINT8 avx512_vl;
    // ECX
    UINT8 prefetchwt1;
    UINT8 avx512_vbmi;
    UINT8 umip;
    UINT8 pku;
    UINT8 ospke;
    UINT8 waitpkg;
    UINT8 avx512_vmbi2;
    UINT8 cet_ss;
    UINT8 gfni;
    UINT8 vaes;
    UINT8 vpclmulqdq;
    UINT8 avx512_vnni;
    UINT8 avx512_bitalg;
    UINT8 tme_en;
    UINT8 avx512_vpopcntdq;
    UINT8 fzm;
    UINT8 la57;
    UINT32 mawau;
    UINT8 rdpid;
    UINT8 kl;
    UINT8 bus_lock_detect;
    UINT8 cldemote;
    UINT8 mprr;
    UINT8 movdiri;
    UINT8 movdir64b;
    UINT8 enqcmd;
    UINT8 sgx_lc;
    UINT8 pks4;
    // EDX
    UINT8 sgx_term;
    UINT8 sgx_keys;
    UINT8 avx512_4vnniw;
    UINT8 avx512_4fmaps;
    UINT8 fsrm;
    UINT8 uintr;
    // reserved
    // reserved
    UINT8 avx512_vp2intersect;
    UINT8 srbds_ctrl;
    UINT8 md_clear;
    UINT8 rtm_always_abort;
    // reserved
    UINT8 rtm_force_abort;
    UINT8 serialize;
    UINT8 hybrid;
    UINT8 tsxldtrk;
    // reserved
    UINT8 pconfig;
    UINT8 lbr;
    UINT8 cet_ibt;
    // reserved
    UINT8 iamx_bf16;
    UINT8 avx512_fp16;
    UINT8 iamx_tile;
    UINT8 iamx_int8;
    UINT8 spec_ctrl;
    UINT8 stibp;
    UINT8 l1d_flush;
    UINT8 arch_capabilities;
    UINT8 core_capabilities;
    UINT8 ssbd;
    /* --- Sub Leaf 1 --- */
    // EAX
    UINT8 sha512;
    UINT8 sm3;
    UINT8 sm4;
    UINT8 rao_int;
    UINT8 amx_vnni;
    UINT8 avx512_bf16;
    UINT8 lass;
    UINT8 cmpccxadd;
    UINT8 archperfmonext;
    UINT8 dedup;
    UINT8 fzrm;
    UINT8 fsrs;
    UINT8 rsrcs;
    // reserved
    // reserved
    // reserved
    // reserved
    UINT8 fred;
    UINT8 lkgs;
    UINT8 wrmsrns;
    UINT8 nmi_src;
    UINT8 iamx_fp16;
    UINT8 hreset;
    UINT8 avx_ifma;
    // reserved
    // reserved
    UINT8 lam;
    UINT8 msrlist;
    // reserved
    // reserved
    UINT8 invd_disable_post_bios_done;
    UINT8 movrs;
    // EBX
    UINT8 ppin;
    UINT8 pbndkb;
    // reserved
    UINT8 cpuid_maxval_lim_rmv;
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
    UINT8 mpsadbw_512;
    // reserved
    UINT8 avx512_rao_fp;
    // ECX
    UINT8 rdt_m_asym;
    UINT8 rdt_a_asym;
    UINT8 reduced_isa;
    // reserved
    UINT8 sipi64;
    UINT8 msr_imm;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    UINT8 ace;
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
    UINT8 avx512_vnni_fp16;
    UINT8 avx512_vnni_int8;
    UINT8 avx512_ne_convert;
    UINT8 avx_vnni_int8;
    UINT8 avx_ne_convert;
    // reserved
    // reserved
    UINT8 iamx_complex;
    // reserved
    UINT8 avx_vnni_int16;
    UINT8 avx512_vnni_int16;
    // reserved
    UINT8 utmr;
    UINT8 prefetchi;
    UINT8 user_msr;
    UINT8 avx512_bf16_ne;
    UINT8 uiret_uif_from_rflags;
    UINT8 cet_sss;
    UINT8 avx10;
    // reserved
    UINT8 apx_f;
    UINT8 sec_tee_attestation;
    UINT8 mwait;
    UINT8 slsm;
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    // reserved
    /* --- Sub Leaf 2 --- */
    // EDX
    UINT8 psfd;
    UINT8 ipred_ctrl;
    UINT8 rrsba_ctrl;
    UINT8 ddpu_u;
    UINT8 bhi_ctrl;
    UINT8 mcdt_no;
    UINT8 uc_lock_disable;
    UINT8 monitor_mitg_no;    
} cpuid_feat7_t;

// Address sizes from leaf 0x80000008.
typedef struct {
    // EBX
    UINT8 clzero;
    UINT8 retired_instr;
    UINT8 xrstor_fp_err;
    UINT8 invlpgb;
    UINT8 rdpru;
    UINT8 xotext; // Playstation 5????
    UINT8 mbe;
    // reserved
    UINT8 mcommit;
    UINT8 wbnoinvd;
    UINT8 lbr_ext_v1;
    // reserved
    UINT8 ibpb;
    UINT8 wbinvd_int;
    UINT8 ibrs;
    UINT8 stibp;
    UINT8 ibrsalwayson;
    UINT8 stibpalwayson;
    UINT8 ibrs_preferred;
    UINT8 ibrs_same_mode_protection;
    UINT8 no_efer_lmsle;
    UINT8 invlpgb_nested;
    UINT8 lbr_tsx;
    UINT8 ppin;
    UINT8 ssbd;
    UINT8 ssbd_legacy;
    UINT8 ssbd_no;
    UINT8 cpppc;
    UINT8 psfd;
    UINT8 btc_no;
    UINT8 ibpb_ret;
    UINT8 branch_sampling;
    // EAX
    UINT32 n_physical_addr_bits;
    UINT32 n_linear_addr_bits;
    UINT32 guest_physical_addr_size;
    // ECX
    UINT32 n_physical_threads; // -1
    UINT32 apic_id_size;
    UINT32 perf_timestep_counter_size;
    // EDX
    UINT32 invlpgb_max_pages;
    UINT32 max_ecx_rdpru;
} cpuid_ext_func2_t;

// SVM features from leaf 0x8000000A (AMD).
typedef struct {
    // EAX
    UINT32 svm_revision;
    // EBX
    UINT32 nasid;
    // EDX
    UINT8  nested_paging;
    UINT8  lbr_virt;
    UINT8  svm_lock;
    UINT8  nrip_save;
    UINT8  tsc_rate_msr;
    UINT8  vmcb_clean;
    UINT8  flush_by_asid;
    UINT8  decode_assists;
    UINT8  pcm_virt;
    UINT8  pause_filter;
    UINT8  emp;
    UINT8  pft;
    UINT8  avic;
    UINT8  v_vmsave_vmload;
    UINT8  vgif;
    UINT8  gmet;
    UINT8  x2avic;
    UINT8  ssscheck;
    UINT8  spec_ctrl;
    UINT8  rogpt;
    UINT8  host_mce_override;
    UINT8  tlbictl;
    UINT8  vnmi;
    UINT8  lbsvirt;
    UINT8  extlvtoffsetfaultchg;
    UINT8  vmcbaddrchkchg;
    UINT8  blt;
    UINT8  ihi;
    UINT8  esi;
    // ECX
    UINT8 gpcmef;
    UINT8 pml;
    UINT8 x2avic_ext;
} cpuid_svm_info_t;

typedef struct {
    // EDX
    UINT8 fpu;
    UINT8 vme;
    UINT8 de;
    UINT8 pse;
    UINT8 tsc;
    UINT8 msr;
    UINT8 pae;
    UINT8 mce;
    UINT8 cx8;
    UINT8 apic;
    // reserved
    UINT8 sep;
    UINT8 mtrr;
    UINT8 pge;
    UINT8 mca;
    UINT8 cmov;
    UINT8 pat;
    UINT8 pse36;
    UINT8 psn;
    UINT8 clfsh;
    UINT8 nx;
    UINT8 ds;
    UINT8 acpi;
    UINT8 mmx;
    UINT8 fxsr;
    UINT8 sse;
    UINT8 sse2;
    UINT8 ss;
    UINT8 htt;
    UINT8 tm;
    UINT8 ia64;
    UINT8 pbe;
    // ECX
    UINT8 sse3;
    UINT8 pclmulqdq;
    UINT8 dtes64;
    UINT8 monitor;
    UINT8 ds_cpl;
    UINT8 vmx;
    UINT8 smx;
    UINT8 est;
    UINT8 tm2;
    UINT8 ssse3;
    UINT8 cntx_id;
    UINT8 sdbg;
    UINT8 fma;
    UINT8 cx16;
    UINT8 xtpr;
    UINT8 pdcm;
    // reserved
    UINT8 pcid;
    UINT8 dca;
    UINT8 sse4_1;
    UINT8 sse4_2;
    UINT8 x2apic;
    UINT8 movbe;
    UINT8 popcnt;
    UINT8 tsc_deadline;
    UINT8 aes_ni;
    UINT8 xsave;
    UINT8 osxsave;
    UINT8 avx;
    UINT8 f16c;
    UINT8 rdrnd;
    UINT8 hypervisor;
} cpuid_feat_t;

typedef struct {
    // EDX
    UINT8 fpu;
    UINT8 vme;
    UINT8 de;
    UINT8 pse;
    UINT8 tsc;
    UINT8 msr;
    UINT8 pae;
    UINT8 mce;
    UINT8 cx8;
    UINT8 apic;
    UINT8 syscall_k6;
    UINT8 syscall;
    UINT8 mtrr;
    UINT8 pge;
    UINT8 mca;
    UINT8 cmov;
    UINT8 pat;
    UINT8 pse36;
    UINT8 ecc_k7;
    UINT8 ecc;
    UINT8 nx;
    UINT8 sem;
    UINT8 mmxext;
    UINT8 mmx;
    UINT8 fxsr;
    UINT8 fxsr_opt;
    UINT8 pdpe1gb;
    UINT8 rdtscp;
    UINT8 rex32_k8;
    UINT8 lm;
    UINT8 tdnowext;
    UINT8 tdnow;
    // ECX
    UINT8 lahf_lm;
    UINT8 cmp_legacy;
    UINT8 svm;
    UINT8 extapic;
    UINT8 cr8_legacy;
    UINT8 abm;
    UINT8 sse4a;
    UINT8 misalignsse;
    UINT8 tdnowprefetch;
    UINT8 osvw;
    UINT8 ibs;
    UINT8 xop;
    UINT8 skinit;
    UINT8 wdt;
    UINT8 tbm0;
    UINT8 lwp;
    UINT8 fma4;
    UINT8 tce;
    UINT8 cvt16;
    UINT8 nodeid_msr;
    // reserved
    UINT8 tbm;
    UINT8 topoext;
    UINT8 perfctr_core;
    UINT8 perfctr_nb;
    UINT8 StreamPerfMon;
    UINT8 dbx;
    UINT8 perftsc;
    UINT8 pcx_l2i_l3;
    UINT8 monitorx;
    UINT8 addr_mask_ext;
    //reserved
} cpuid_ext_feat_t;

// Topology from leaf 0x0B (Intel extended topology).
typedef struct {
    UINT8  level_type;            // 1=Thread, 2=Core, 3=Module, etc.
    UINT32 logical_per_level;
    UINT32 level_number;
    UINT32 x2apic_id;
    UINT8  valid;
} cpuid_topology_t;

// Frequency info from leaves 0x15 and 0x16.
typedef struct {
    UINT32 tsc_denominator;
    UINT32 tsc_numerator;
    UINT64 tsc_freq_hz;
    UINT32 base_freq_mhz;         // Leaf 0x16 EAX
    UINT32 max_freq_mhz;          // Leaf 0x16 EBX
    UINT32 bus_freq_mhz;          // Leaf 0x16 ECX
} cpuid_freq_info_t;

// Main CPU info structure.
typedef struct {
    char     vendor[13];
    char     brand[49];

    UINT32 psn[3];

    UINT32 max_basic_leaf;
    UINT32 max_extended_leaf;


    _CpuidProcInfo   proc;
    cpuid_feat_t        features;
    cpuid_ext_feat_t    features_ext;
    cpuid_feat7_t       feat7;

    _CpuidMwaitInfo  mwait;
    _CpuidThermalInfo thermal;
    
    cpuid_ext_func2_t   addr_size;
    cpuid_freq_info_t   freq;


    // --- Embedded sub-structures ---

    _CpuidCacheInfo  caches[8];      // Leaf 4: usually 4-6 caches
    UINT32            cache_count;
    cpuid_topology_t    topo[8];        // Leaf 0x0B: usually 2-3 levels
    UINT32            topo_count;
    cpuid_svm_info_t    svm;            // Leaf 0x8000000A

    UINT8             has_mwait;
    UINT8             has_thermal;
    UINT8             has_svm;
    UINT8             has_topology;
} _CpuInfo, *_PCpuInfo;

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif