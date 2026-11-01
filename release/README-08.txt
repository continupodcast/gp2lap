GP2Lap F1 HUD 0.8

INSTALLATION
Replace GP2LAP.EXE and EDITOR.html in your existing installation. Keep your
personal GP2LAP.CFG and season logos. Launch GP2LAP.EXE in SVGA 640x480.
Open your current CFG/project in EDITOR.html, adjust options, export and
restart GP2Lap to apply settings. Do not replace GP2.EXE.

OPTIONAL TYRE COMPOUND SYSTEM
The master switch remains off in the normal bundled configuration.
Tyre Compound System groups compounds, AI strategies, varied patterns,
player strategy selection, live pit tyre choice and compound HUD badges.
Require a compound change defaults ON. Turn it OFF to allow same-compound
strategies. Normal calculated races still require a pit stop; Quickrace can
run without stops. StrategyAutoSeed controls only this strategy distribution.

PLAYER CONTROLS
Race strategy starts on Default (GP2's loaded plan). Use arrows to navigate
by visible button position and Enter or Space to press. Minus/plus changes
values only when activated. Restore Default, Ok and Cancel share one row.
The selector never shows strategy ranking or time estimates.
In the live native pit menu, use GP2's arrows for Fuel laps / Stops to go /
Tyre and its native Fire confirmation. Pit service keeps running.
Existing HUD controls 3 / TAB / 4 / 5 / 6 / 7 are retained.

DOCUMENTATION
BACKLOG.md: completed work, remaining work and relative effort.
CHANGELOG-0.8.md: changes since verified GitHub release v0.56 and source audit.
source/: matching source and original licence notices.
validation/: retained functional tests and build records; not needed to play.
This is the only installation README in the packaged release.

LIMITS
Single human driver; strategy state not persisted in saved games. Current
model uses the measured tyre/fuel calibration and 27 seconds per stop, not
universal circuit calibration. No validated traffic/power model is included.
Historical research and removed experiments are not enabled game features.

STATUS
Version 0.8 closes T20.7 with unused merge/diagnostic material removed.
DOS compilation and controlled tests pass. Final compound-rule toggle and
cleanup need an in-game check. Prepared locally; not published to GitHub.
