# AIR Arcade MakeCode compatibility work

This folder is the first, portable runtime layer for bringing a supported subset of Microsoft MakeCode Arcade games to AIR Arcade.

## Important compatibility boundary

A MakeCode Arcade `.uf2` file is firmware for a supported ARM-based board. It is **not** a game file that an ESP32-S3 Cardputer can execute. AIR Arcade will import a MakeCode project/source export and translate supported game operations into AIR Arcade bytecode instead.

The intended pipeline is:

1. Import a MakeCode Arcade project export.
2. Read its TypeScript source and asset data.
3. Translate supported MakeCode APIs into AIR Arcade bytecode.
4. Run that bytecode in the same portable VM on the desktop simulator and Cardputer.
5. Report unsupported APIs instead of silently pretending they work.

## This first milestone

- Adds a small C++ bytecode VM with a 240 x 135 logical screen.
- Provides sprite movement, button-state reads, simple variables, score display, and rectangle collision checks.
- Keeps the VM independent of NucleoOS headers so it can be compiled and tested on a PC first.
- The next milestone is the MakeCode TypeScript/project importer; this VM alone does not yet import or run arbitrary MakeCode projects.

## NucleoOS integration

NucleoOS documents native apps as signed `.nap` modules compiled for its Xtensa ABI. The VM is designed as a portable C++ component that can later be wrapped by a NucleoOS native-app adapter. Do not copy a MakeCode `.uf2` onto the Cardputer.

## Current status

**Prototype foundation — not yet MakeCode-project compatible.** The initial opcode set is intentionally small. Each added MakeCode API will be documented with a compatibility test.
