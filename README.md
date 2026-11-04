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

## Modular development

The project has **18 modular branches: 16 feature modules, one shared core and one fully integrated development branch**. This structure lets developers reuse selected F1 HUD features and build custom combinations with their required dependencies.

### Stable version and integrated development

| Branch or release | Purpose | Validation status |
| --- | --- | --- |
| [v0.8 release](https://github.com/continupodcast/gp2lap/releases/tag/v0.8) / [`f1-hud`](https://github.com/continupodcast/gp2lap/tree/f1-hud) | Stable 0.8 implementation, with its previous source structure. | Tested in GP2 by the project maintainer. |
| [`modular/core`](https://github.com/continupodcast/gp2lap/tree/modular/core) | Shared GP2Lap platform, configuration, hooks, telemetry, rendering primitives and compatibility interfaces. No F1 feature modules selected. | DOS compilation checked. |
| [`modular/integrated`](https://github.com/continupodcast/gp2lap/tree/modular/integrated) | All 16 feature modules combined. This is the branch for continued modular development. | DOS builds and host logic checks passed; in-game validation remains pending. |

`f1-hud` and `modular/integrated` are intended to provide the same 0.8 functionality. They have different source layouts and are not interchangeable validation references: equivalent behavior inside GP2 still needs to be confirmed for the modular build. The published v0.8 release remains the stable reference.

### Feature branches

All modules require `modular/core`. The last column lists additional **direct** dependencies; their own dependencies are included transitively.

| Branch | Feature | Additional dependencies |
| --- | --- | --- |
| [`modules/timing-tower`](https://github.com/continupodcast/gp2lap/tree/modules/timing-tower) | Race and qualifying timing towers, fastest-lap banners. | Core only |
| [`modules/driver-panels`](https://github.com/continupodcast/gp2lap/tree/modules/driver-panels) | Driver identity and current-gap panels. | Core only |
| [`modules/sectors`](https://github.com/continupodcast/gp2lap/tree/modules/sectors) | Sector cards and microsector comparisons. | Core only |
| [`modules/circuit-maps`](https://github.com/continupodcast/gp2lap/tree/modules/circuit-maps) | Full and local circuit maps. | Core only |
| [`modules/pit-timing`](https://github.com/continupodcast/gp2lap/tree/modules/pit-timing) | Live pit-signal cards. | Core only |
| [`modules/native-captions`](https://github.com/continupodcast/gp2lap/tree/modules/native-captions) | Native caption fonts, layout and visibility filters. | Core only |
| [`modules/camera-controls`](https://github.com/continupodcast/gp2lap/tree/modules/camera-controls) | Enhanced cameras and external pit-camera access. | Core only |
| [`modules/fuel-drain`](https://github.com/continupodcast/gp2lap/tree/modules/fuel-drain) | Optional selected-car fuel drain. | Core only |
| [`modules/quickrace-grid`](https://github.com/continupodcast/gp2lap/tree/modules/quickrace-grid) | Custom Quickrace ordering and grid-menu presentation. | Core only |
| [`modules/tyre-compounds`](https://github.com/continupodcast/gp2lap/tree/modules/tyre-compounds) | Per-car compound infrastructure and tyre telemetry. | Core only |
| [`modules/tyre-compound-hud`](https://github.com/continupodcast/gp2lap/tree/modules/tyre-compound-hud) | Compound badges and tyre-age display. | `tyre-compounds`, `timing-tower` |
| [`modules/race-strategy-engine`](https://github.com/continupodcast/gp2lap/tree/modules/race-strategy-engine) | Measured compound model, strategy generation and AI assignment. | `tyre-compounds` |
| [`modules/player-race-strategy`](https://github.com/continupodcast/gp2lap/tree/modules/player-race-strategy) | Editable pre-race player strategy and player service tracking. | `race-strategy-engine` |
| [`modules/pit-compound-selector`](https://github.com/continupodcast/gp2lap/tree/modules/pit-compound-selector) | Live keyboard-driven compound selection in the native pit menu. | `player-race-strategy` |
| [`modules/combined-qualifying`](https://github.com/continupodcast/gp2lap/tree/modules/combined-qualifying) | Friday/Saturday combined qualifying display. | `timing-tower` |
| [`modules/hud-editor`](https://github.com/continupodcast/gp2lap/tree/modules/hud-editor) | Browser editor for configuration; no DOS runtime dependency. | Core only |

For example, `pit-compound-selector` requires `player-race-strategy`, which requires `race-strategy-engine`, which requires `tyre-compounds`. The compound HUD requires both compounds and the timing tower; combined qualifying requires the timing tower.

Each feature branch contains its required dependency branches. Feature source payloads live under `modules/<name>/`, and selection files live under `modules/enabled/`. Some internal helpers and data models remain in the shared core to preserve the existing C/assembly interfaces. Existing extensions inherited from rremedio remain in that platform; they are not counted as newly separated modules. Original license terms and attribution continue to apply; see the retained [0.8 provenance audit](https://github.com/continupodcast/gp2lap/blob/modular/integrated/CREDITS-v0.8.md) and [license](LICENSE.md).

### Build and combine modules

Module selection happens **at build time**. Install Open Watcom, set `WATCOM` to its installation directory, and use Python 3 to run the build wrapper from a modular branch:

```sh
python tools/modules.py list
python tools/modules.py build --output ../my-build
```

Use `python3` instead of `python` where required. The wrapper resolves dependencies, rejects missing module sources and forces a complete rebuild when switching profiles. On `modular/integrated`, any available subset can be selected explicitly:

```sh
python tools/modules.py build --modules circuit-maps,driver-panels --output ../maps-and-driver
python tools/modules.py build --modules pit-compound-selector --output ../pit-selector
```

To assemble a custom source branch, start from `modular/core` and merge the desired `modules/*` branches with their included dependencies. Shared configuration switches remain optional: including a module does not force its gameplay setting on. A CFG cannot activate a module omitted from the executable. The editor is included in build output when `hud-editor` is selected.

The modular delivery passed **21 clean DOS build configurations and 14 host logic checks**, plus branch dependency checks and a reproducible integrated rebuild. These results do not replace testing inside GP2, particularly save/load, accelerated time, native pit menus and camera hooks.

See [MODULARITY.md on the integrated branch](https://github.com/continupodcast/gp2lap/blob/modular/integrated/MODULARITY.md) for the full dependency and build guide, and [the modular delivery report](https://github.com/continupodcast/gp2lap/blob/modular/integrated/MODULAR-DELIVERY.md) for the build matrix. The delivery report records the original local handoff; the modular branches have since been published to this repository.

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


