GP2Lap F1 HUD v0.35 - evaluation build

CHANGES SINCE 0.34
FASTEST LAP: use GP2Lap's original fastest-caption suppression hook,
extended to the active race HUD (both themes). The v0.34 private drawing
buffer workaround has been removed. Other message hooks are unchanged.
The modern fastest banner has larger, horizontally centred driver names.
CURRENT GAP: positions vertically centred, names lowered and side margins
increased. Timing data and millisecond precision are unchanged.
Map: Left Shift + 6 changes zoom ONLY while the full circuit map is visible:
1x / 1.25x / 2x. The local car map always retains its original scale.
Key 6 still cycles full circuit / local car map / hidden.

INSTALLATION
Replace GP2LAP.EXE in your GP2 directory.
If keeping your existing GP2LAP.CFG, set this under [At The Line]:
atlNoFastestLap = 1
Restart GP2Lap. This setting hides the original fastest-lap caption when
the race tower is enabled; the HUD banner remains visible. Setting it to
0 keeps the original caption. The original At The Line behaviour remains.
The supplied CFG and the editor's initial configuration use 1.
Importing an older CFG into the editor preserves that file's setting.
Keep your season data and logos. The ZIP includes CFG, editor and source.

VALIDATION
Compiled with Open Watcom. Automated renderer and map tests passed.
The suppression-hook branches preserve registers, flags and stack in CPU
emulation. In-game RetroArch validation of v0.35 remains pending.

Editor import fix: player flags 0x80 and 0x40 are separated from CarId using mask 0x3F. GP2LAP.EXE is unchanged.


V0.35 COMPACT SECTOR CARD UPDATE
TV: sector/microsector card is bottom-centre, 210x84 native pixels.
Onboard: upper-centre two-row card, 280x50 native pixels, with abbreviation,
logo, main time, reference and sector/microsector bars.
Qualifying/practice keys 4 and 5 replace each other.
Open Watcom build and host rendering/toggle tests pass. Gameplay validation
in RetroArch remains required. CFG and editor are preserved from the supplied ZIP.
