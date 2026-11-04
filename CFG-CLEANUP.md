# Modular CFG cleanup

This correction updates all modular branch CFG templates and all 21 packaged build profiles.

- Remove inactive/default entries for `JamTextureExtraMB`, `ExtraTrackSegments`, `ExtraTrackFileKB`, `PitSpeedKmh`, `AILowHpLaunchFix`, `AILaunchFadeKmh`, `AIBankingRoll`, and the commented `SeasonOverrides` example. Drop their empty sections.
- Activate `SvgaRcr1`, `SvgaRcr2`, and `SvgaRcr3` at the existing shipped value of `9999000`. VGA settings remain unchanged.
- Clean new editor exports and imported older CFG defaults. Preserve explicitly enabled legacy options, custom RCR values, unknown settings and the user's HUD/strategy data.
- Set the native `PitSpeedKmh` default to **0**. Its previous internal default was 80, despite the distributed CFG explicitly disabling it. With the new EXE, removing that CFG line keeps the native pit speed unchanged.

**Install the corrected EXE and CFG together.** Pairing the cleaned CFG with the old executable could activate its previous internal pit-speed default. The updated editor is also included in `builds/integrated/`.

The inherited implementation of these optional features remains in the source. This cleanup removes unused defaults from the distributed configuration; it does not claim those inherited implementations were absent.

Validation: 21 clean DOS profile builds, 15 host check groups (including CFG policy and editor export regression tests), all 21 output CFGs inspected for removed defaults and active SVGA RCR settings, exact module dependency closure and reproducible integrated rebuild. Actual behavior inside GP2 still requires user testing. The published stable v0.8 tag is unchanged. These cleanup commits have not been pushed to GitHub from this session.
