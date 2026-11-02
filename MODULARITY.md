# GP2Lap F1 HUD — modular source branches

Based on the published `v0.8` commit `c6b46529859aad903bc1fc10ce8f5e4afe4349c9`. The published tag and `f1-hud` reference are preserved. This is a development layout; it does not replace the stable release.

## What is separated

`modular/core` supplies the GP2Lap host, memory structures, original extensions, configuration, shared telemetry, palette/font renderer primitives and ABI-compatible hook symbols. It supplies no selected F1 modules. Disabled module entry points compile as no-ops, and disabled compound/strategy capabilities override CFG requests before native patches are installed. Headless profiles skip HUD telemetry acquisition.

Each `modules/<name>` branch adds its source payload under `modules/<name>/`, its manifest and a selection file under `modules/enabled/`. Dependencies are merged into the branch. Feature function bodies are separate include fragments compiled in the original translation units, preserving the legacy C/assembly calling conventions and shared private state. No duplicate native hook installation is introduced.

This is source/build modularity, not a DLL/plugin loader. The common core is required. Internal helpers, data models and some telemetry remain shared; these branches are not standalone patches for vanilla GP2Lap or rremedio's different base. Existing upstream extensions in the 0.8 platform remain available and are not claimed as newly separated modules. Their provenance and license are preserved in `CREDITS-v0.8.md` and `LICENSE.md`.

| Branch | Added function | Required feature branches |
| --- | --- | --- |
| `modules/timing-tower` | Race/qualifying towers and fastest lap | — |
| `modules/driver-panels` | Driver name and current gap | — |
| `modules/sectors` | Sector and microsector cards | — |
| `modules/circuit-maps` | Local and full maps | — |
| `modules/pit-timing` | Pit signal/timing cards | — |
| `modules/native-captions` | Native caption font and visibility | — |
| `modules/camera-controls` | Enhanced cameras and pit views | — |
| `modules/fuel-drain` | Optional fuel drain | — |
| `modules/quickrace-grid` | Quickrace custom grid and grid screen | — |
| `modules/tyre-compounds` | Per-car compounds and tyre telemetry | — |
| `modules/tyre-compound-hud` | Compound badges and tyre age | compounds, tower |
| `modules/race-strategy-engine` | AI strategy generation/assignment | compounds |
| `modules/player-race-strategy` | Editable player pre-race strategy | strategy engine |
| `modules/pit-compound-selector` | Native live pit compound menu | player strategy |
| `modules/combined-qualifying` | Combined Friday/Saturday display | tower |
| `modules/hud-editor` | Browser configuration editor | — |

`modular/integrated` merges every feature branch. Configuration switches remain optional: a module being compiled does not force its gameplay option on.

## Compile

Install Open Watcom and set `WATCOM` to its directory. Python 3 is required for dependency resolution and reproducible clean builds. From the repository root:

```sh
python3 tools/modules.py list
python3 tools/modules.py build --output ../my-build
```

The branch's `modules/enabled/*.module` files select its capabilities. To build a different subset from an integrated checkout:

```sh
python3 tools/modules.py build --modules pit-compound-selector --output ../pit-only
python3 tools/modules.py build --modules circuit-maps,driver-panels --output ../maps-and-driver
python3 tools/modules.py build --modules "" --output ../core-only
```

The first command automatically includes the strategy engine, player strategy and compounds. Missing module sources, unknown names and dependency cycles are errors. Each invocation deletes previous object files because the historical makefile does not track header dependencies. Do not bypass this wrapper when switching profiles. The generated header is deliberately untracked; a fresh checkout requires configuration before using the historical makefiles.

Build output contains a DOS EXE, a CFG with omitted gameplay modules disabled, license/credits and a JSON capability/hash record. The editor is included only when selected. An omitted module cannot be enabled by changing the CFG. Modern and 90s rendering themes remain shared configuration choices.

## Combine branches

Start from `modular/core`, then merge whichever `modules/*` branches are wanted. Branch additions use distinct filenames; no shared profile file is edited by feature branches. Required source branches travel with each feature branch. To remove a module from a custom composition, remove its `.module` selection file; dependent modules will still require it. Rebuild through the wrapper.

```sh
git switch -c my-composition modular/core
git merge modules/circuit-maps modules/pit-compound-selector
python3 tools/modules.py build --output ../my-composition-build
```

## Validation and limits

`python3 tools/test_modules.py` runs the integrated host regression suite (GCC and Node.js required). It exercises actual extracted player/menu bodies, compound enforcement, strategy editing/navigation, distribution, palette handling, grid parsing, editor CFG round-trip and disabled renderer surfaces. Run it from `modular/integrated`, which contains every test dependency. It does not run GP2.

The delivery includes clean DOS builds for the base, each feature branch, the integrated branch and mixed subsets, with SHA-256 records. Branch merges are verified. Compiler warnings about unused parameters in disabled ABI stubs are expected; they are not missing-symbol or link errors.

Actual GP2 runtime tests are still needed after this structural change, especially save/load, accelerated time, menu banking and camera hooks. Build and host-test success are not a claim of in-game validation. The existing 0.8 limitations on circuit calibration, strategy persistence and multiple human players are unchanged.

## Credits

`CREDITS-v0.8.md` is the retained audit of the original 0.8 bytes, not a byte-identity audit of this modular development tree. Modular changes additionally alter shared integration paths, including `src/gridcap.c` and `src/timing/convert.inc`, and extract feature bodies into `modules/`. No ownership claim is made over upstream source. The original project license continues to apply.
