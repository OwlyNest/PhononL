#include <MM/Heap.H>
#include <Lib/Lib.H>

INT StrCmp(PCCHAR S1, PCCHAR S2) {
    while (*S1 && (*S1 == *S2)) {
        S1++;
        S2++;
    }
    return *(PUCHAR)S1 - *(PUCHAR)S2;
}

UINT32 AtoI(PCCHAR Str) {
    UINT32 Result = 0;
    while (*Str >= '0' && *Str <= '9') {
        Result = Result * 10 + (*Str - '0');
        Str++;
    }
    return Result;
}

VOID ItoA(INT N, PCHAR Str) {
    INT I = 0;
    INT IsNeg = 0;

    if (N < 0) {
        IsNeg = 1;
        N = -N;
    }

    do {
        Str[I++] = (N % 10) + '0';
        N /= 10;
    } while (N > 0);

    if (IsNeg) {
        Str[I++] = '-';
    }
    Str[I] = '\0';

    // reverse
    for (INT J = 0; J < I / 2; J++) {
        CHAR T = Str[J];
        Str[J] = Str[I - 1 - J];
        Str[I - 1 - J] = T;
    }
}

PCHAR StrCat(PCHAR Dest, PCCHAR Src) {
    PCHAR Original = Dest;

    while (*Dest) {
        Dest++;
    }

    while (*Src) {
        *Dest++ = *Src++;
    }

    *Dest = '\0';
    return Original;
}

PCHAR StrnCat(PCHAR Dest, PCCHAR Src, SIZE_T N) {
    PCHAR Original = Dest;

    while (*Dest) {
        Dest++;
    }

    while (*Src && N) {
        *Dest++ = *Src++;
        N--;
    }

    *Dest = '\0';
    return Original;
}

PCHAR StrCpy(PCHAR Dest, PCCHAR Src) {
    PCHAR Original = Dest;
    while ((*Dest++ = *Src++));
    return Original;
}

PCHAR StrChr(PCCHAR Str, INT C) {
    while (*Str) {
        if (*Str == (CHAR)C) {
            return (PCHAR)Str;
        }
        Str++;
    }
    return NULL;
}

PCHAR StrDup(PCCHAR Str) {
    SIZE_T Len = StrLen(Str);
    PCHAR Copy = (PCHAR)ExAllocatePool(Len + 1);
    if (Copy) {
        MemCpy(Copy, Str, Len);
        Copy[Len] = '\0';
    }
    return Copy;
}

SIZE_T StrLen(PCCHAR Str) {
    SIZE_T Len = 0;
    while (Str[Len]) {
        Len++;
    }
    return Len;
}

INT StrnCmp(PCCHAR S1, PCCHAR S2, SIZE_T N) {
    while (N && *S1 && (*S1 == *S2)) {
        S1++;
        S2++;
        N--;
    }
    if (N == 0) {
        return 0;
    }

    return *(PUCHAR)S1 - *(PUCHAR)S2;
}

PCHAR StrnCpy(PCHAR Dest, PCCHAR Src, SIZE_T N) {
    PCHAR Original = Dest;

    while (N && *Src) {
        *Dest++ = *Src++;
        N--;
    }
    while (N--) {
        *Dest++ = '\0';
    }
    return Original;
}

SIZE_T StrnLen(PCCHAR S, SIZE_T MaxLen) {
    SIZE_T Len = 0;
    while (Len < MaxLen && S[Len]) {
        Len++;
    }
    return Len;
}

PCHAR StrChrNul(PCCHAR S, INT C) {
    while (*S) {
        if (*S == (CHAR)C) {
            return (PCHAR)S;
        }
        S++;
    }
    return (PCHAR)S;
}

PCHAR StrRChr(PCCHAR S, INT C) {
    PCCHAR Last = NULL;
    while (*S) {
        if (*S == (CHAR)C) {
            Last = S;
        }
        S++;
    }
    return (PCHAR)Last;
}

PCHAR StrnDup(PCCHAR S, SIZE_T N) {
    SIZE_T Len = StrnLen(S, N);
    PCHAR Copy = (PCHAR)ExAllocatePool(Len + 1);
    if (!Copy) {
        return NULL;
    }
    for (SIZE_T I = 0; I < Len; I++) {
        Copy[I] = S[I];
    }
    Copy[Len] = '\0';
    return Copy;
}

PVOID MemSet(PVOID Dest, INT Val, SIZE_T Len) {
    PUCHAR Ptr = (PUCHAR)Dest;
    while (Len-- > 0) {
        *Ptr++ = (UCHAR)Val;
    }
    return Dest;
}

PVOID MemCpy(PVOID Dest, PCVOID Src, SIZE_T Len) {
    PUCHAR D = (PUCHAR)Dest;
    const unsigned char * S = (const unsigned char *)Src;
    while (Len--) {
        *D++ = *S++;
    }
    return Dest;
}

INT MemCmp(PCVOID S1, PCVOID S2, SIZE_T N) {
    const PUCHAR A = (PUCHAR)S1;
    const PUCHAR B = (PUCHAR)S2;

    for (SIZE_T I = 0; I < N; I++) {
        if (A[I] != B[I]) {
            return A[I] - B[I];
        }
    }
    return 0;
}

PVOID MemMove(PVOID Dest, PCVOID Src, SIZE_T N) {
    PUCHAR D = (PUCHAR)Dest;
    const PUCHAR S = (PUCHAR)Src;

    if (D == S) {
        return Dest;
    }

    if (D < S) {
        for (SIZE_T I = 0; I < N; I++) {
            D[I] = S[I];
        }
    } else {
        for (SIZE_T I = N; I > 0; I--) {
            D[I - 1] = S[I - 1];
        }
    }

    return Dest;
}

/* Just blatantly copied from the gcc libiberty */
PVOID MemChr(PCVOID SrcVoid, INT C, SIZE_T Length) {
    const unsigned char *Src = (const unsigned char *)SrcVoid;

    while (Length-- > 0) {
        if (*Src == (UCHAR)C) {
            return (PVOID)Src;
        }
        Src++;
    }
    return NULL;
}