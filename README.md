# GP2Lap F1 HUD
--------------

This repository is a personal fork of GP2Lap that includes the GP2Lap F1 HUD modification.

The mod adds a native modern-style Formula 1 overlay to Grand Prix 2, including:

- Race and qualifying timing towers
- TV and onboard layouts
- Driver cards and team colors
- Full-track map with colored car markers
- Microsector timing and record tracking
- Pit-stop indicators and position-change indicators
- Battle camera support
- Fuel-drain tools for removing unused cars
- HUD configuration editor
- Support for up to 14 teams in qualifying and practice sessions

The original GP2Lap functionality and keyboard commands remain available.

This repository contains the source code, build tools, tests, documentation, and editor files. Executable builds and ready-to-use packages are distributed through the GitHub Releases section.

## Project status

The F1 HUD is under active development and testing with Grand Prix 2 running through RetroArch and DOSBox Pure.

## License

This fork is based on the original GP2Lap project. See the original license and project documentation for licensing details.


What is GP2Lap
--------------

GP2Lap is a real-time extension for GP2.
Its main functions include logging information for online competitions, like processor occupancy, and showing on-screen information such as the current standings, fastest lap times, and a track map.

GP2Lap should work with all (language) versions of GP2.
If you have trouble getting it to work with your version then please let us know.

Under Windows XP or newer, GP2 can be run inside [DOSBox](http://www.dosbox.com/).
A whole copy of GP2 including a DOSBox configuration can be found on [OldGames.sk](http://www.oldgames.sk/en/game/grand-prix-2/download/5004/).

Building GP2Lap
---------------

GP2Lap can be cross compiled to DOS/4G with [Open Watcom](http://www.openwatcom.org/).

### On Windows

The included Makefile requires Windows XP/Vista/7/8, with the Watcom/BINNT directory in the search PATH.
It can be run with nmake.exe and it will use wcc386.exe, wasm.exe and wlink.exe to build gp2lap.exe.
To run gp2lap.exe in DOSBox, dos4gw.exe needs to be present too. It can be copied from the Watcom/BINW diretory.

The very first time, use "nmake init" to create output directories.

Use plain "nmake" to create "out\gp2lap.exe".

Use "nmake publish" to create a zip and documentation in "pub\".

### On Linux

Open Watcom v2 also ships Linux-hosted versions of the same tools (wcc386, wasm, wlink and wmake),
so GP2Lap can be built natively on Linux using `makefile.lin`. The Windows `makefile` is left untouched.

Install Open Watcom v2 (for example under `~/watcom`) and load its environment; Open Watcom provides
an `owenv.sh` that sets `WATCOM` and `PATH`:

    . ~/watcom/owenv.sh

Then build from the repository root with wmake:

    wmake -f makefile.lin init     # first time only: create the output directories
    wmake -f makefile.lin          # build out/gp2lap.exe

As on Windows, dos4gw.exe must be present next to gp2lap.exe to run it, and the result runs under
DOSBox (tested with DOSBox-X and DOSBox-Staging on Linux).


