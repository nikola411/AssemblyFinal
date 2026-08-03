"""
Wraps the C++ unit-test binaries so each C++ test shows up as its own item in
the VS Code Testing tab.

Discovery is driven by .vscode/settings.json (unittest, start dir ./tests,
pattern test*.py). This file lives in ./tests so it is found without any
settings change; the C++ sources/binaries stay in ./tests/unit.

Each suite is built and run once at import time; every "<name> ... PASS/FAIL"
line becomes a Python test method, so results appear per-test in the tree.
"""

import re
import subprocess
import unittest
from pathlib import Path

UNIT_DIR = (Path(__file__).parent / "unit").resolve()

ANSI = re.compile(r"\x1b\[[0-9;]*m")
LINE = re.compile(r"^\s+(\w+)\s+\.\.\.\s+(PASS|FAIL)(?::\s*(.*))?\s*$")

# suite name -> (make target, produced binary)
SUITES = {
    "linker": ("build-linker", "linker_test"),
    "elf":    ("build",        "elf_test"),
    # "unload" (UnloadLinkableTest.cpp) is a manual dump program that prints
    # tables, not a PASS/FAIL harness, so there is nothing to parse — skipped.
}


def build_and_run(make_target, binary):
    build = subprocess.run(["make", make_target], cwd=UNIT_DIR,
                           capture_output=True, text=True)
    if build.returncode != 0:
        return None, (build.stderr or build.stdout)

    run = subprocess.run([str(UNIT_DIR / binary)], cwd=UNIT_DIR,
                         capture_output=True, text=True)

    results = {}
    for raw in run.stdout.splitlines():
        m = LINE.match(ANSI.sub("", raw))
        if m:
            name, status, msg = m.group(1), m.group(2), m.group(3)
            results[name] = (status == "PASS", msg or "")
    return results, run.stdout


def make_case(suite, make_target, binary):
    results, output = build_and_run(make_target, binary)
    attrs = {}

    if results is None:
        def test_build(self, output=output):
            self.fail("build failed:\n" + output)
        attrs["test_build"] = test_build
    elif not results:
        def test_parsed(self, output=output):
            self.fail("no tests parsed from output:\n" + output)
        attrs["test_parsed"] = test_parsed
    else:
        for name, (ok, msg) in results.items():
            def test(self, ok=ok, msg=msg):
                self.assertTrue(ok, msg)
            attrs[name] = test

    return type("Test_" + suite, (unittest.TestCase,), attrs)


# expose one TestCase per suite at module level so discovery finds them
for _suite, (_target, _binary) in SUITES.items():
    globals()["Test_" + _suite] = make_case(_suite, _target, _binary)
