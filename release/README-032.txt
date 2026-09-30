GP2Lap F1 HUD v0.32

CHANGES IN 0.32
Timing history is preserved when a previously tracked car temporarily
leaves the main-track geometry, including the pit approach before IN PIT.
No crossings are interpolated through that unavailable route. Existing
pit labels and pit-stop counting still use the original game signals.
Comparisons use the available common points in the latest six-point window:
six, five, four, three, two or one. With none, the last measured value for
that exact pair is held where pit recovery applies. A pair with no known
measurement can still show a dash. Rejoining begins with a fresh anchor.
The 90s DIFFERENCE panel receives the interpolated average in milliseconds;
the race towers continue to display one decimal. These are calculated
milliseconds, not an independent native timing measurement.
F1GAPS.LOG and F1HUD.LOG both identify version 0.32.
The 90s tower deliberately has no permanent fastest-lap badge.

BUILD AND TEST
Place the supplied Open Watcom toolchain at ../toolchains/watcom relative
to source/. From source/: . tools/watcom-env.sh
Then run: wmake -a -f makefile.lin
Run the release regression suite with: sh tools/test_032.sh
Tests need a C compiler, Python 3 with Pillow, and Node.js. The editor tests
use synthetic LE data rather than external GP2.EXE files. Legacy individual
test scripts remain archived and may require their original fixtures.
EDITOR.html is included both at package root and in source/release/.

INSTALLATION
Back up GP2LAP.EXE and your personalized GP2LAP.CFG. Replace GP2LAP.EXE.
Keep your existing CFG to retain your drivers, teams, colours and logos.
The package includes an updated default GP2LAP.CFG and EDITOR.html.
No modification to GP2.EXE is required. Launch as usual.

1990s BROADCAST THEME (new)
A second graphics theme, selected in GP2LAP.CFG or in the editor
(In-game controls > HUD graphics). Applied when GP2Lap starts.

    [F1 Controls]
    HudTheme = 0     ; modern F1 graphics (default, unchanged)
    HudTheme = 1     ; late-1990s broadcast graphics

Without the HudTheme line the modern theme is used, so existing CFG files
keep working exactly as before.

What changes with HudTheme = 1 (Futura Condensed, yellow position boxes,
cyan and lime accents, dark translucent panels):
- Race tower: RACE POSITIONS / CURRENT LAP header, full surnames, intervals
  in lime, IN PIT in cyan, OUT dimmed. Keys and tower modes are the same;
  the header names the active mode. Position changes colour the box green
  or red with an arrow for the configured time.
- Driver plate: yellow position box, first name and surname, team name in
  cyan and the car number in a cyan oval.
- DIFFERENCE (race): focused car against the car ahead, or leader against
  second. Hidden while either car is in the pits or out, and while no
  measured gap exists.

KEYS IN THE 90s THEME
- TV view, race: plate and DIFFERENCE share the bottom-centre slot, which
  ends at line 380 because GP2 only displays the first 388 lines of its
  image in TV view (0.30 drew them lower and they were invisible). Key 5
  cycles driver plate -> DIFFERENCE -> off, like key 4 with sectors and
  microsectors.
- Qualifying and onboard: key 5 shows or hides the plate (onboard at the top
  centre, as before). DIFFERENCE is only shown in the race, in TV view.
- TAB cycles the race tower modes, as in the modern theme.
- Modern theme: every key works exactly as in 0.28.
- Qualifying tower and card: same data and sector colours, 90s styling.
Maps, microsectors, fuel drain and pit counting are unchanged in both themes.
Both themes share the updated gap calculation described above.

NOTES
Colours are matched to the nearest entries of GP2's 256-colour palette, so
they can differ slightly from the reference design. The race tower in the
90s theme is 132 pixels wide (modern: 104). GP2Lap's own "Fastest Lap by"
at-the-line message can be disabled with atlNoFastestLap = 1 if it
duplicates the new banner.

VALIDATION
DOS compilation, the existing synthetic tests (gaps, pits, card, compact
tower, CFG, editor) and new 90s tests (CFG parsing, element bounds for 1 to
26 cars, every focus position, TV/onboard) pass. Actual RetroArch gameplay
validation remains pending.
