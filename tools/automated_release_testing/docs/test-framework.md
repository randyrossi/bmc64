# Writing and organising hardware tests

The runner discovers one concrete `Test` class per `tests/<suite>/test_*.py`
file. A suite is a folder, not a manually maintained registration list.
Ordinary Python modules and assets in that folder are suite support files,
not executable tests.

```text
framework/                 Lifecycle, shared rig, discovery, cleanup and results
  case.py                  BmcTest: hooks, context shortcuts and shared steps
  rig.py                   The Pico, UART logger and capture every test shares
  runner.py                Preflight, per-test execution and the run report
utils/                     CLI, setup wizard and video preview implementations
hardware/                  HTTP, Pico/UART and HDMI capture
support/                   Helpers shared across suites
tests/
  boot/
    base.py                Boot-suite steps (ROM reader, screen waits, typing)
    layouts.py             Layout data type, not machine expectations
    screen.py              Screen waits and pane detection
    screen_text.py         ROM-font reader
    test_c64.py            C64 expectations, ROM/capture settings and test flow
    test_c128.py
    ...
  basic/
    diagnostic.py          Signal decoding and audio measurements
    test_c64_default.py    C64 guest program and full diagnostic assertions
unit_tests/                Offline tests of the above, never hardware discovery
```

Suite packages have `__init__.py`. Nested folders can form sub-suites;
selecting a parent includes its descendants. Keep helpers specific to a suite
inside that suite. Move a helper into `support/` (or a step into `BmcTest`)
only when it is actually shared across suites.

Keep the procedure in the test file. Its `before_test()` and `run_test()` should
show what happens and in which order, including machine-specific expectations,
inputs and capture settings. Helpers implement reusable operations; a suite base
must not hide the entire test in an inherited `run_test()`. There is no need
for a suite base class when only one test uses the procedure.

## Commands

```sh
./run_tests                          # every test
./run_tests --list                   # stable IDs, descriptions and suite names
./run_tests boot                     # six boot/typing tests
./run_tests basic                    # C64 diagnostic
./run_tests pet-boot-typing
./run_tests boot c64-default
```

Repeated selections are deduplicated. Tests run sequentially, not in
parallel; the Pi, UARTs and capture device are exclusive resources. They run
in `order` (ties use suite/name): boot tests 10-60, then the diagnostic at 70.

Every test names its `machine`. `--manifest` (default `suite_profiles.json`)
maps each machine to its SD-card test profile; it is not a registry of Python
test files. A test whose machine is not in the manifest is left out of
`--list` and cannot be selected.

Root-level `run_tests`, `run_smoke`, `run_video_test` and
`configure_test_setup` are small executable launchers. Their implementations
live in `utils/`; default manifest, results and saved setup paths resolve
relative to the test directory, not `utils/` or the caller's working directory.

Without `--execute`, direct CLI use only calls each test's `prepare_test()`
(for example generating the C64 diagnostic) and reports `PREPARED`; it never
opens the rig or calls the hardware hooks. The `run_tests` launcher supplies
`--execute`.

## A test file

```python
from .base import BootTypingTest
from .layouts import Layout


class Test(BootTypingTest):
    name = "c64-boot-typing"
    description = "C64: boot into BASIC and verify USB typing"
    machine = "C64"
    order = 10
    capture_size = (1280, 720)
    layout = Layout(
        columns=40, rows=25, input_row=6, channel=0, inverted=False,
        font_path="/C64/chargen", font_offset=0,
        screen=(
            (1, "    **** COMMODORE 64 BASIC V2 ****"),
            (3, " 64K RAM SYSTEM  38911 BASIC BYTES FREE"),
            (5, "READY."),
        ),
    )

    def before_test(self) -> None:
        super().before_test()
        self.prepare_screen_reader(self.layout, self.capture_size, token_length=8)
        self.start_profile(fresh=True)
        self.open_native_capture()

    def run_test(self) -> None:
        self.assert_boot_screen()
        self.wait_for_keyboard()
        self.type_usb_token()
        self.assert_typed_token()
        self.assert_profile_unchanged()

    def after_test(self) -> None:
        super().after_test()
```

Machine-specific banners, font offsets and dimensions live alongside the
sequence, not in a central machine registry. The PET test also specifies its
borderless frame geometry.

The C64 diagnostic derives directly from [`BmcTest`](../framework/case.py)
and implements its own steps in
[`test_c64_default.py`](../tests/basic/test_c64_default.py). Only low-level
signal helpers live in [`diagnostic.py`](../tests/basic/diagnostic.py).

Use a unique lowercase hyphenated `name` as the stable CLI ID. The containing
folder supplies `suite`; do not register new tests in the CLI. Modules must
have no hardware side effects when imported, because discovery and `--list`
import test definitions without opening the rig.

## What a test can use

`BmcTest` gives each test shortcuts to its context:

| Attribute | Meaning |
| --- | --- |
| `self.api`, `self.pico`, `self.capture`, `self.uart` | The shared rig (`uart` is `None` without `--bmc-uart`) |
| `self.profile` | This test's resolved profile: `machine`, `id`, `name`, `volume` |
| `self.output` | This test's evidence directory |
| `self.report` | This test's `result.json` contents |
| `self.timeout` | The per-assertion deadline (`--timeout`) |
| `self.health()` | Raises if the capture failed or the UART logged a critical message during this test; call it in every wait |
| `self.stage(name)` | Records the current stage, reported with any failure |
| `self.passed(check)` | Records a passed assertion in `checks` |

and the steps every suite shares:

- `start_profile(fresh=True)` boots the test's profile and verifies its ID
  and core. `fresh=True` always restarts BMC64, even when the profile is
  already running; `fresh=False` keeps a running profile with the right core.
- `open_capture(size, directory="capture-1", video_size=None)` reopens the
  capture into the test's evidence, analysed at `size`. Call it after
  `start_profile()`: a capture left open across an HDMI mode change records
  corrupt frames.
- `wait_for_keyboard()` waits for the Pico to remount after a restart.

## Lifecycle

The runner does this once per run:

1. **Preflight**: checks HTTP status and resolves every selected test's profile
   (and network setting with `--network`), before opening anything else.
2. **Opens the rig**: Pico UART (waiting until the keyboard is ready), the
   optional BMC64 UART logger and the HDMI capture. A Pi that reboots while
   the capture device is closed comes up with no HDMI picture, so the
   capture is running before the first profile switch.
3. Runs each test, as below.
4. If a test fails, checks that HTTP control still answers. If it does not,
   the remaining tests are NOT_RUN without further switches.
5. Checks final HTTP status and closes the rig.

For each test it calls:

1. `before_test()` for test-specific setup.
2. `run_test()` for assertions; raise `TestFailure(category, message)` on a
   known failure rather than returning a success/failure boolean.
3. `after_test()`, even when setup or assertions fail or are interrupted.
4. Registered cleanup callbacks, in reverse registration order.
5. The teardown every test gets: Pico `RELEASE`, a check of this test's UART
   findings and the last captured frame as evidence.
6. Saves the test's `result.json`.

`after_test()` must tolerate partially completed setup. Call the parent hooks
when overriding suite setup/teardown.

The framework owns hook execution; test files should not wrap their own
lifecycle or catch failures to turn them into a PASS. Cleanup errors are
recorded separately, do not erase the primary failure, and prevent PASS.
Interruption saves the current result and marks later tests NOT_RUN.

Register cleanup immediately after acquiring a resource, before the next
operation that could fail. The framework closes registered resources even
if an overridden `after_test()` fails or does not call its parent:

```python
def before_test(self) -> None:
    resource = acquire_resource()
    self.context.register("close resource", resource.close)
    self.resource = resource
```

Optional `prepare_test()` performs offline-only preparation; it must never
access the rig, and the rig shortcuts raise if it tries.

## Run evidence

Each run writes one ignored `results/<UTC-time>-<run-id>/` folder:

- `result.json`: overall status, setup, and one entry per selected test
  (PASS, FAIL with stage/category/error, NOT_RUN with the reason, or
  PREPARED) plus any run-level cleanup errors.
- `status-before.json` / `status-after.json` and `resolved-profiles.json`.
- `capture.mkv`, `ffmpeg.log`, `capture-command.json`: the startup recording
  segment, until the first test reopens the capture.
- `pico.log`: one line per Pico command with its reply and round-trip time,
  keys shown by name (`KEY V`, `KEY release`), Pico restarts and errors, and
  a marker at each test start. `pico.raw` is the exact TX/RX transcript with
  request IDs.
- With `--bmc-uart`, `bmc64-uart.log` (readable, marked at each test start),
  `bmc64-uart.raw` (exact bytes) and `uart-findings.json` for the whole run.
- One numbered directory per test (`01-c64-boot-typing/`, ...), with its own
  `result.json`, recording segments and assertion evidence. See the
  [profile suite](profile-suite.md#evidence-and-diagnosis) and
  [diagnostic](smoke-test.md#evidence-and-cleanup) guides for their contents.

Test timestamps and UART findings use the same PC monotonic clock, so a
test's result can be matched against the continuous UART log.

## Offline validation

From the repository root:

```sh
python3 -m unittest discover \
  -s tools/automated_release_testing/unit_tests \
  -t tools/automated_release_testing -p 'test_*.py' -v
```

The top-level argument makes the local packages importable. Do not use generic
unittest discovery on the hardware `tests/` folder: its files follow the custom
`BmcTest` lifecycle, not `unittest.TestCase`.

Offline checks cover discovery, selectors, hook failures, partial setup,
cleanup failures and interruption, alongside the mocked rig, transport,
capture and screen regression coverage. Recorded-ROM checks still require
`BMC64_ROM_ROOT`; they skip explicitly when it is absent.

No offline result substitutes for a run on the connected rig.
