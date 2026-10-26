# Combined fixes export — r5382

## Install for game testing

**Use only with a matching Realism Invictus r5382 installation and its XML/data.** This DLL retains r5382 serialization; it does not upgrade save formats. Compatibility with r5484, newer revisions or arbitrary saves is not claimed.

1. Close Civilization IV completely.
2. Back up your working `Assets/CvGameCoreDLL.dll` and saves. Prefer a separate r5382 test installation and copied saves.
3. Copy **only** this package's `files/CvGameCoreDLL.dll` to the mod's `Assets/CvGameCoreDLL.dll`.
4. Do **not** copy source files, patch or entire `files/` directory into Assets.
5. Test with copied saves. Restore the backed-up DLL if problems occur.

Packaging did not install the DLL, rebuild sources or change the shipped Assets DLL.

## Contents

```text
pathfinder-bug-fix-export-r5382/
  README.md
  SHA256SUMS
  fixes-on-r5382.patch
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

`SHA256SUMS`: 11 entries covering README, patch, DLL and eight sources; paths relative to package root. Verify before use:

```bash
sha256sum -c SHA256SUMS
```

DLL: **5,984,256 bytes**, PE32 x86, large-address-aware. SHA-256:

```text
509b5a511085b57c3abd8712357f6cb793f37382d293e936e3d612dcf09773cf
```

## Scope / adaptation

All nine original Combined Solution fixes retained:

1. Pathfinder dimension/allocation guards and safe initialization/reset.
2. Persistent UI finder storage; fresh search state per refresh.
3. Early map setup allocation; clear references before teardown.
4. Segmented node allocation and partial-allocation cleanup.
5. Throwing allocation policy in optional memory-manager overrides.
6. Replay minimap array deletion.
7. Save map-dimension and array-count validation.
8. Throttled virtual-address/contiguous-region paging pressure with hysteresis.
9. Bounds/cost/tree checks and iterative path-cost propagation.

Seven modified sources + new `RISaveValidation.h`. Adapted hunks from r5484 onto official r5382; not wholesale r5484 source replacement. Retained r5382 naval heuristic, UI, nuclear/combat/yield/working-city/aid behavior and game/team serialization. No serialization-order upgrades, enum renumbering or XML/Python fixes imported.

## Apply source patch with SVN

For developers only; unnecessary for installing supplied DLL. Use a separate clean **r5382** checkout, rooted above `realism/`. Review status and dry-run before applying; never apply again to the already-patched working tree.

```bash
svn info
svn status
svn patch --dry-run "D:/210_Projects/25_Civ4_RI/civ4mods-code_r5382/temp/pathfinder-bug-fix-export-r5382/fixes-on-r5382.patch"
svn patch "D:/210_Projects/25_Civ4_RI/civ4mods-code_r5382/temp/pathfinder-bug-fix-export-r5382/fixes-on-r5382.patch"
svn status
svn diff -- realism/bts/trunk/mod/GameCore/CvGameCoreDLL
```

Patch paths start `realism/...`, not `a/` or `b/`: **no `--strip 1`**. Check new header scheduling. No commit performed.

Private pathfinder layout changed: full clean rebuild required if rebuilding. Existing VC++ Toolkit 2003 / Platform SDK / Boost 1.32.0 / Python24 toolchain retained. Disable automatic deployment with `ASSETS_DIR=`. No project-profile lane build command configured.

## Provenance / verification

Authoritative deliverable: sibling `../pathfinder-bug-fix-r5382/`; official source baseline:

`https://svn.code.sf.net/p/civ4mods/code/realism/bts/trunk/mod/GameCore/CvGameCoreDLL@5382`

Upstream `SHA256SUMS` verified before packaging. DLL, patch and eight sources copied byte-for-byte from that verified deliverable. Isolated r5382 SVN patch application reproduces all eight packaged sources; originals checked against saved official baseline. Packaging adds no source changes and performs no rebuild.

Prior build/test evidence retained in authoritative deliverable's `logs/`, `checks/`, `baseline/`, `build.sh` and `verify.py`:

- Fresh Release compile/link: **106 objects**, PCH and resource; zero errors. Three existing C4244 narrowing warnings in unchanged player sources.
- VC++ 2003 propagation harness: 20 recursive/iterative comparisons, 20,000-node chain, invalid child count and self-link.
- Extracted storage-method harness: invalid dimensions, reuse/reset, injected directory/partial-block allocation failure cleanup, retry/release.
- 144 dimension pairs against 64-bit oracle; eight optional-array count cases.
- Map/plot/replay stream operations unchanged; game/team sources byte-identical to r5382; naval heuristic retained. 23 count guards and three replay array-deletion sites checked.
- Optional `USE_MEMMANAGER` allocator object compiled separately. Normal Release does not enable it; no allocator-mode link/runtime claim.

## Known unresolved runtime issue / limits

**Original r5484 package was reported to show `Memory allocation Failure: Bad allocation` when selecting a building or unit for production. Cause remains unproven. This r5382 backport does not claim to fix that failure; no tracing enhancement is included.**

No in-game/save-load/multiplayer regression pass. Harness allocation-failure injection is not game allocator testing. Segmentation reduces contiguous-allocation demand, not total memory; persistent UI storage retains memory. Paging cannot guarantee allocation success. Invalid dimensions/counts intentionally terminate the game; uncaught allocation failure can also terminate it. Keep the known-working DLL and original saves available.
