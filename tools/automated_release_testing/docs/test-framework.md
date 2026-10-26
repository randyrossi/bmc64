# Writing and organising hardware tests

The runner discovers one concrete `Test` class per `tests/<suite>/test_*.py`
file. A suite is a folder, not a manually maintained registration list.
Ordinary Python modules and assets in that folder are suite support files,
not executable tests.

```text
framework/                 General lifecycle, discovery, cleanup and results
utils/                     CLI, setup wizard and video preview implementations
hardware/                  HTTP, Pico/UART and HDMI capture
support/                   Helpers shared across suites
tests/
  boot/
    base.py                Shared boot operations and rig lifecycle
    session.py             Shared boot-suite rig session and preflight
    layouts.py             Layout data type, not machine expectations
    screen.py              Screen waits and pane detection
    screen_text.py         Existing ROM-font reader
    test_c64.py            C64 expectations, ROM/capture settings and test flow
    test_c128.py
    ...
  basic/
    diagnostic.py          Signal decoding and audio measurements
    test_c64_default.py    C64 guest program, setup and full diagnostic assertions
unit_tests/                Offline tests of the above, never hardware discovery
```

Suite packages have `__init__.py`. Nested folders can form sub-suites;
selecting a parent includes its descendants. Keep helpers specific to a suite
inside that suite. Move a helper into `support/` only when it is actually
shared across suites.

Keep the procedure in the test file. Its `before_test()` and `run_test()` should
show what happens and in which order, including machine-specific expectations,
inputs and capture settings. Helpers implement reusable operations; a suite base
must not hide the entire test in an inherited `run_test()`. There is no need
for a suite base class when only one test uses the procedure.

## Commands and compatibility

```sh
./run_tests                          # all seven existing tests
./run_tests --list                   # stable IDs, descriptions and suite names
./run_tests boot                     # six boot/typing tests
./run_tests basic                    # C64 diagnostic
./run_tests pet-boot-typing
./run_tests boot c64-default
```

Individual IDs are unchanged. Repeated selections are deduplicated. Tests run
sequentially, not in parallel; the Pi, UARTs and capture device are exclusive
resources. The existing order is preserved: boot cases in manifest order,
then the diagnostic. The `order` class attribute sets default discovery order
for other tests (ties use suite/name).

`--manifest` still selects the profile mapping. It is not a registry of Python
test files. As before, a subset manifest limits available boot tests.
Root-level `run_tests`, `run_smoke`, `run_video_test` and
`configure_test_setup` are small executable launchers. Their implementations
live in `utils/`; default manifest, results and saved setup paths still resolve
relative to the test directory, not `utils/` or the caller's working directory.
Saved `test_setup.json` needs no changes. The setup wizard also recognises and
upgrades the old generated launcher when reconfiguring an existing rig.
New configurations record `network` and verify profile settings during
preflight. The setup wizard opens its own HDMI preview, updates the installed
profiles and verifies the final C64 connection before saving; this is an
interactive hardware operation, unlike offline test preparation.

Without `--execute`, direct CLI use only prepares results and generates the
C64 diagnostic when selected; it never calls hardware before/run/after hooks.
The `run_tests` launcher supplies `--execute`, as it did previously.

## A test file

```python
from .base import BootTypingTest
from .layouts import Layout


class Test(BootTypingTest):
    name = "c64-boot-typing"
    description = "C64 BASIC boot and USB typing"
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
        self.start_profile()
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

Every boot test owns this sequence; the repeated ROM decoding, frame waits,
USB operations and profile checks are shared methods. Machine-specific banners,
font offsets and dimensions live alongside the sequence, not in a central
machine registry. The PET test also specifies its borderless frame geometry.

The standalone C64 diagnostic derives directly from
[`BmcTest`](../framework/case.py) and implements its own `run_test()`.
Its guest program, preparation, setup, timing and assertions are in
[`test_c64_default.py`](../tests/basic/test_c64_default.py).
Only low-level signal helpers remain in
[`diagnostic.py`](../tests/basic/diagnostic.py).

Use a unique lowercase hyphenated `name` as the stable CLI ID. The containing
folder supplies `suite`; do not register new tests in the CLI. Modules must
have no hardware side effects when imported, because discovery and `--list`
import test definitions without opening the rig.

## Lifecycle

For an executing test the framework calls:

1. `before_test()` for test-specific preparation.
2. `run_test()` for assertions; raise `TestFailure(category, message)` on a
   known failure rather than returning a success/failure boolean.
3. `after_test()` even when preparation or assertions fail or are interrupted.
4. Registered cleanup callbacks, in reverse registration order.
5. Per-case evidence and result persistence.

`after_test()` must tolerate partially completed setup. Call the parent hooks
when overriding suite setup/teardown.

The framework owns hook execution; test files should not wrap their own
lifecycle or catch failures to turn them into a PASS. Cleanup errors are
recorded separately, do not erase the primary failure, and prevent PASS.
Interruption saves the current result and prevents later tests from running.

The context contains arguments, output directory, result dictionary and
registered resources:

```python
def before_test(self) -> None:
    resource = acquire_resource()
    self.context.register("close resource", resource.close)
    self.resource = resource
```

Register cleanup immediately after acquiring a resource, before the next
operation that could fail. The framework closes registered resources even
if an overridden `after_test()` fails or does not call its parent. Use the
context's `health()` callback during waits and retain explicit stage names
in `context.report["stage"]`.

Optional `prepare_test()` performs offline-only preparation; it must never
access the rig. It is separate from hardware hooks so offline discovery and
preparation are safe by default.

## Shared sessions and preserved hardware behaviour

The default `BmcTest.run_suite()` runs standalone cases with their own resource
lifetimes. A suite base can override this method to share a rig session.
The boot base delegates to its neighbouring `session.py`, which retains the
existing profile preflight, shared Pico/UART connections, failure recovery,
per-case evidence and manifest order.

Each boot test still:

- Downloads the same ROM and uses the same `ScreenText` assertions/thresholds.
- Starts/reboots the profile with the existing verification rules.
- Records through profile changes, then reopens capture at the core's native
  resolution and allows the existing bounded unreadable-frame reopen.
- Verifies two fresh boot frames, USB token input and two fresh typed frames.
- Retries final HTTP identity, checks UART and releases input.

The C64 diagnostic retains its existing capture-before-profile-switch sequence,
PRG, video counter, tone, Space response and silence checks. PET remains the
supplied 40-column 3016 profile. This restructure does not qualify other
profiles, fonts, models or capture settings.

Results retain the existing boot-suite/per-case and standalone-diagnostic
formats and evidence layout. This keeps the structural refactor separate
from changing the hardware assertions or evidence consumers.

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
cleanup failures and interruption, alongside the existing mocked rig,
transport, capture and screen regression coverage. Recorded-ROM checks still
require `BMC64_ROM_ROOT`; they skip explicitly when it is absent.

No offline result substitutes for a post-refactor run on the connected rig.
