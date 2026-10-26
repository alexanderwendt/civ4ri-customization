# Combined fixes export — r5484

Short on how to run: 
1. Backup your `CvGameCoreDLL.dll` by renaming to CvGameCoreDLL.dll.old
2. Copy `files/CvGameCoreDLL.dll` to your asset directory. 
3. Start the game

## Scope

Folder name says pathfinder; package contains **all nine Combined Solution fixes**, including allocation, replay, save validation and graphical paging changes. Authoritative deliverable: `temp/Combined_Solution_r5469_savefix`. No r5469 diagnostic DLL or unfinished allocation-trace work included.

## Contents

```text
pathfinder-bug-fix-export/
  README.md
  SHA256SUMS
  fixes-on-r5484.patch
  files/
    CvGameCoreDLL.dll
    realism/bts/trunk/mod/GameCore/CvGameCoreDLL/
      CvGameCoreDLL.cpp
      CvGameInterface.cpp
      CvMap.cpp
      CvPlot.cpp
      CvReplayInfo.cpp
      KmodPathFinder.cpp
      KmodPathFinder.h
      RISaveValidation.h
```

`files/CvGameCoreDLL.dll` is separate testing artifact, not installed Assets DLL. Eight exported sources match patch output and compiled/validated source snapshot exactly. Original encodings/newlines retained. `SHA256SUMS` covers patch, DLL, eight source files and this README; paths relative to export root.

## Exact baseline / compatibility correction

Baseline: official SVN **r5484**, source directory:

`https://svn.code.sf.net/p/civ4mods/code/realism/bts/trunk/mod/GameCore/CvGameCoreDLL@5484`

User-supplied working DLL matched official r5484 byte-for-byte:

`2579024827dbd47febff759928394740ee6701a295a1b9a76205ba2bdb5f85b2`

Earlier replacement used r5469 source, incompatible with supplied save. r5484 serializes `CvGame::m_iSeparatismEra` after nuclear-winter value; r5469 omits it. Reader therefore entered map data four bytes early, interpreting `(flag,width,height)` as `(0,0,210)`. Actual save's map is **210 x 90**. r5484 also serializes two additional team-state arrays: `m_abCanTechTransfer`, `m_abCouldEverTechTransfer`.

Correction: rebuilt nine fixes on exact working r5484 baseline. No hardcoded map dimensions, guessed stream skips or weakened overflow checks. Patch does **not** upgrade r5469 to r5484; obtain correct baseline first. No universal compatibility with r5382, r5469, latest revisions or other save formats claimed. Keep Assets/XML consistent with working r5484 installation.

## Changes

| Fix | Change | Files |
|---|---|---|
| 1 | Validate pathfinder dimensions/allocation; initialize and reset state safely | `KmodPathFinder.cpp/.h` |
| 2 | Persistent UI pathfinder storage, fresh search state per refresh | `CvGameInterface.cpp`, `KmodPathFinder.cpp/.h` |
| 3 | Early UI storage preparation; clear references before map teardown | `CvMap.cpp`, `KmodPathFinder.cpp/.h` |
| 4 | Segmented node allocation; paired cleanup including partial allocation | `KmodPathFinder.cpp/.h` |
| 5 | Mandatory allocation failure policy in optional memory-manager overrides | `CvGameCoreDLL.cpp` |
| 6 | Match replay minimap array allocation with array deletion | `CvReplayInfo.cpp` |
| 7 | Validate map dimensions and saved array counts before unsafe allocations | `CvMap.cpp`, `CvPlot.cpp`, new `RISaveValidation.h` |
| 8 | Throttled virtual-address pressure/contiguous-region checks and hysteresis | `CvPlot.cpp` |
| 9 | Bounds/cost/tree checks and iterative path-cost propagation | `KmodPathFinder.cpp/.h` |

Seven modified files, one new header. No XML/Python changes, enum renumbering or serialization-order edits relative to r5484. Pathfinder private layout changed: **full clean rebuild required**, not only changed objects. Normal Release does not enable `USE_MEMMANAGER`; FIX 5 overrides are present in source but inactive in supplied DLL. Both modes object-compiled during verification.

## Apply with SVN

Use a clean, separately backed-up **r5484 checkout**, rooted so `realism/bts/trunk/mod/GameCore/CvGameCoreDLL` exists underneath it. Run from that checkout root; substitute actual export path if moved. Do not blindly overwrite unrelated changes or apply to a different revision.

```bash
svn status
svn patch --dry-run --strip 1 "D:/210_Projects/25_Civ4_RI/civ4mods-code_r5469/pathfinder-bug-fix-export/fixes-on-r5484.patch"
# Continue only after reviewing a conflict-free dry run against r5484.
svn patch --strip 1 "D:/210_Projects/25_Civ4_RI/civ4mods-code_r5469/pathfinder-bug-fix-export/fixes-on-r5484.patch"
svn status
svn diff -- realism/bts/trunk/mod/GameCore/CvGameCoreDLL
```

Check new `RISaveValidation.h` is present and review SVN's scheduling/status. No commit performed or implied. SVN dry-run commands above are instructions; package verification used isolated `git apply --check`/application against exported r5484 sources, then byte-compared all eight results.

Alternative: exported sources are exact patch outputs for r5484; use only when baseline and unrelated edits have been checked. Do not copy entire `files/` tree into Assets.

## Build / install

Existing Civ4 Makefile/toolchain retained: VC++ Toolkit 2003, Platform SDK, Boost 1.32.0, Python24. Fresh Release PCH, all translation units and resource linked together. Before rebuilding, back up prior outputs, clean all relevant objects/PCH, configure dependencies and disable Makefile's automatic Assets copying (`ASSETS_DIR=`) for testing. No profile lane build command configured.

For supplied binary testing: close game, preserve working DLL and saves, copy **only** `files/CvGameCoreDLL.dll` into a separate matching r5484 test installation's `Assets/CvGameCoreDLL.dll`. Do not change original saves; load a copy. Production Assets DLL was not replaced during preparation.

Supplied DLL: 5,894,144 bytes, PE32 x86, large-address-aware.

SHA-256:

`e737951f033a19d3eba6a78e9bd2d53ce64ad554b1229655daba24085404749c`

Verify package from export root:

```bash
sha256sum -c SHA256SUMS
```

## Actual verification

- Fresh Release compile/link passed: **107 objects**, PCH and resource; no warning/error lines in build log.
- Actual save-boundary fixture, compiled with VC++ 2003: corrected reader gets 210x90; old r5469 reader reproduces 0x210 rejection; corrected game-tail/AI/map-prefix writer round-trips fixture bytes exactly.
- **144** dimension-boundary cases passed against 64-bit arithmetic oracle; guards unchanged.
- **20** object compiles passed: six changed translation units + four direct consumers, each with/without `USE_MEMMANAGER`.
- Pathfinder harness passed: 20 recursive/iterative comparisons, 20,000-node chain, invalid child count and self-link.
- Relevant compiled serializer routines match working r5484 DLL after normalizing relocated image addresses; not whole-program equivalence proof.
- Packaging: patch lists exactly exported eight sources; isolated application to r5484 reproduces them byte-for-byte; copies match authoritative source/build/validation hashes and DLL. Packaging performs **no rebuild**.

Full logs and reproduction scripts remain in authoritative `temp/Combined_Solution_r5469_savefix`; this export contains requested patch/docs/sources/DLL, not test intermediates.

## Known unresolved runtime issue / limits

**User reported `Memory allocation Failure: Bad allocation` when selecting a building or unit for production with this exact save-compatible DLL. Cause not yet proven. This package retains that tested binary; it is not a fix for the allocation failure.**

Requested stack-trace enhancement is **NOT included**. Allocation-trace investigation was unfinished when export was requested; no trace handler or matching trace-build symbols claimed.

Pathfinder segmentation reduces contiguous allocation demand, not total memory; persistent UI storage retains memory. Paging checks do not guarantee allocation success. Existing r5484 paging behavior outside nine fixes remains. Invalid save dimensions/counts intentionally terminate game; uncaught allocation failure may terminate game. No verified multiplayer, failure-injection or full in-game regression pass. Keep known-working DLL available; use copied saves/test installation only.
