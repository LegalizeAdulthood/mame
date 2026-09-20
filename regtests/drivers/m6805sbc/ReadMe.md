# MC146805E2 SBC Integration Tests

This directory contains black-box integration tests for the
`src/mame/homebrew/m6805sbc.cpp` driver.  The tests launch a built MAME
executable, load a generated test ROM into the SBC ROM socket, and use MAME's
Lua interface to inspect CPU-visible hardware behavior.

## What Is Tested

The first probe, `lua/memory_map.lua`, validates the external RAM decode added
for the SBC driver:

- Writes to `$0002`, `$0003`, `$000A`, `$000F`, `$0080`, `$0110`, and `$0800`
  persist.
- A write to the 4 KB ROM socket does not persist.
- An internal low address such as `$0004` is not accidentally modeled as
  external RAM.

Additional probes should stay focused on one hardware behavior at a time.

## Test ROM Fixtures

The Python harness generates a 4 KB ROM fixture at run time.  The fixture is
only test input for the emulator.  Its contents are not board ROM contents and
must not be added to the driver ROM definition.

The generated ROM contains a small idle loop at `$1000`, a known byte used for
the ROM write-protection check, and reset-vector bytes that point to `$1000`.

## Running Locally

The checked-in MAME test harness is the Python runner.  From the MAME
repository root, run it with a built emulator executable:

```bat
py regtests\drivers\m6805sbc\m6805sbctest.py --executable C:\code\mamedev\build-mame\vs2026\bin\x64\Release\m6805sbc.exe
```

On non-Windows systems, use Python directly and pass the MAME executable:

```sh
python regtests/drivers/m6805sbc/m6805sbctest.py --executable ./mame
```

## GitHub CI

The Linux GitHub Actions workflow runs these tests only for the full `mame`
matrix job.  This keeps the integration coverage close to the implementation
while avoiding extra cost on every platform/compiler build.

## Temporary Output

The harness writes generated ROMs, rendered Lua scripts, MAME configuration
directories, and probe result files under a temporary directory.  These files
are deleted when the test process exits and should not be committed.
