# GP2Lap F1 HUD v0.8 — source provenance

The original GP2Lap authors and contributors retain credit for the underlying project.
This version incorporates source from https://github.com/rremedio/gp2lap, branch dev.
Reference commit checked: `67cb8e8be5fc52dc9fec3e1e52cefefc8f6934f2`.

The comparison covers the 43 paths listed in the retained experimental merge record.
It compares their current 0.8 bytes against that reference; it is not a claim to have
recovered the exact original import commit, nor does it assign sole authorship of
whole files. The reference is the source branch, not a downloaded binary release.

## Files identical to the checked upstream reference (35)

- `src/cartex.h`
- `src/cartex.c`
- `src/roster.h`
- `src/aiaccel.h`
- `src/carhelmet.c`
- `src/sessions.h`
- `src/calendar.h`
- `src/magicd.c`
- `src/pitcrew.c`
- `src/teamphys.c`
- `src/pitstops.c`
- `src/aibank.h`
- `src/sessions.c`
- `src/aiaccel.c`
- `src/gridcap.h`
- `src/drvdata.c`
- `src/teamphys.h`
- `src/override.h`
- `src/carhelmet.h`
- `src/pitstops.h`
- `src/roster.c`
- `src/pitcrew.h`
- `src/override.c`
- `src/calendar.c`
- `src/jamidcap.c`
- `src/aibank.c`
- `src/jamidcap.h`
- `src/carshape.c`
- `src/magicd.h`
- `src/drvdata.h`
- `src/carshape.h`
- `src/timing/rlaptime.c`
- `src/timing/codehook.inc`
- `src/timing/qatl.inc`
- `src/timing/timinstd.h`

## Files differing from the checked upstream reference (8)

- `makefile.lin`
- `makefile`
- `src/gridcap.c`
- `src/lammcall.asm`
- `src/gp2hook.c`
- `src/cfgdefin.inc`
- `src/timing/rbehind.c`
- `src/timing/convert.inc`

These integration/build paths differ in the F1 HUD version. Changes include HUD
integration, configuration, tyre handling and the custom Quickrace grid. A binary
difference alone does not assign the authorship of every changed line.

## Project-specific development

The measured tyre/fuel performance model, strategy optimization and distribution,
pre-race strategy editor and live pit compound selector were developed after the
integration. They build on the inherited game-access and compound infrastructure.
The compound system is therefore credited as a mixture of incorporated upstream
code and project-specific adaptations, rather than a wholly independent rewrite.

Unused merge utilities/reports and retired diagnostic output were removed from the
0.8 package. Active source and copyright/licence notices remain. Preserving or
modifying a source file does not remove its original attribution.
