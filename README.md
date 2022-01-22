# Cover

Cover is a framework library to easily create test suites for C (and thus C++).

Cover is declarative and uses the global symbol `cover_suite` to find the user-defined set of tests.
It defines its custom set of signal handlers and calls appropriate reporting backends to wrap the tests results.
It enforces reports under a resilient executable which would not just crash when a test fails under heavy circumstances (eg. stack overflows).

## Keep in mind

Individual test suites should aim for minute-duration testing (which could be unit tests, functional tests...).
Cover makes efforts to ensure total completion of the test suite, and executes test cases sequentially.
Tests should not trigger process termination, and should rely on cover to fail/terminate, thus allowing
cover to correctly call finalization triggers, complete reports and resource cleanups gracefully.

## Examples

A set of simple examples is provided with this repository, three examples showing how to use some features of cover:
- assert: Using assertions inside tests to test invariants.
- finit: How to define an initialization and/or a finalization hook to setup or free resources for the test suite.
- signal: Shows how cover catches common failure signals to enforce test suite completion and meaningful reportings.

A cover test executable must explicitly activate its reporting backends when called.
For example, and if you are in your build directory and built the assert example:
```
examples/assert -output - # Activate the 'output' backend with option '-', which means use standard output
```

## Backends

Cover dynamically finds its backends, when an argument is provided on the command line (like `-output` in the previous examples).
When detected, it looks up the associated `cover_backend_<backend>_init` initialization and `cover_backend_<backend>_report` reporting function symbols.
The initialization is done before the `init` hook, and thus is allowed to fail before the test suite execution begins.
It allows defining custom reporting backends alongside any project's test suite without having to recompile libcover or install anything.

- [x] Fancy/colorful output.
- [ ] TAP (Test anything protocol).
- [ ] JUnit XML report.

## Configure, build and install

Meson is used to configure, build and install the library:

```sh
meson setup build
meson compile -C build
meson install -C build
```

## Documentation

To do.
