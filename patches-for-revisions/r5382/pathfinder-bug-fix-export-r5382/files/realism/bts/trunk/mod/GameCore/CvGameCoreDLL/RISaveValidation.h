#pragma once
// RI FIX 7 BEGIN - shared policy for malformed/incompatible serialized array sizes.
// These void read interfaces expose no recoverable failure result to the EXE.
// Do not return with a partially consumed stream or unwind into an unknown EXE handler.
inline void RIInvalidSave(const char* detail)
{
    OutputDebugStringA(detail);
    MessageBoxA(NULL, detail, "Realism Invictus: invalid or incompatible save", MB_OK | MB_ICONERROR);
    ExitProcess(ERROR_INVALID_DATA);
}

inline void RIValidateSaveCount(int actual, int expected, const char* detail)
{
    // Zero represents an absent optional array. Nonzero arrays are indexed by
    // the current player/team/XML domain, so smaller allocations are also unsafe.
    if (actual < 0 || (actual != 0 && actual != expected))
        RIInvalidSave(detail);
}
// RI FIX 7 END