import subprocess
import sys
import tempfile
import difflib
from pathlib import Path

GREEN  = "\033[32m"
RED    = "\033[31m"
YELLOW = "\033[33m"
CYAN   = "\033[36m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
RESET  = "\033[0m"

RUNNER_DIR = Path(__file__).parent
E2E_DIR    = RUNNER_DIR / "e2e"

# Map suite name -> executable relative to project root
SUITE_EXECUTABLES = {
    "asm":      RUNNER_DIR.parent / "asembler",
    "linker":   RUNNER_DIR.parent / "linker",
    "emulator": RUNNER_DIR.parent / "emulator",
    "elf":      RUNNER_DIR / "e2e" / "elf" / "elf_runner",
}

# Map suite name -> input file name (default: test.asm)
SUITE_INPUT_FILES = {
    "elf": "test.elf",
}


def get_suites(filter_suite=None):
    suites = []
    if E2E_DIR.exists():
        for item in sorted(E2E_DIR.iterdir()):
            if item.is_dir():
                if filter_suite is None or item.name == filter_suite:
                    suites.append(item.name)
    return suites


def get_tests(suite):
    suite_dir = E2E_DIR / suite
    tests = []
    for item in sorted(suite_dir.iterdir()):
        if item.is_dir():
            tests.append(item)
    return tests


def run_single_test(executable, test_dir, input_filename="test.asm"):
    asm_file      = test_dir / input_filename
    expected_file = test_dir / "expected.txt"

    if not asm_file.exists():
        return "skip", f"missing {input_filename}", None

    if not expected_file.exists():
        return "skip", "missing expected.txt  (run --update to generate)", None

    with tempfile.NamedTemporaryFile(suffix=".txt", delete=False) as tmp:
        actual_path = Path(tmp.name)

    result = subprocess.run(
        [str(executable), "-o", str(actual_path), str(asm_file)],
        capture_output=True, text=True
    )

    if not actual_path.exists() or actual_path.stat().st_size == 0:
        actual_path.unlink(missing_ok=True)
        return "error", result.stderr.strip()[:200], None

    actual   = actual_path.read_text()
    expected = expected_file.read_text()
    actual_path.unlink(missing_ok=True)

    if actual == expected:
        return "pass", None, None

    diff = list(difflib.unified_diff(
        expected.splitlines(keepends=True),
        actual.splitlines(keepends=True),
        fromfile="expected", tofile="actual", n=2
    ))
    return "fail", None, diff


def print_diff(diff):
    for line in diff[:30]:
        if line.startswith("+") and not line.startswith("+++"):
            print(f"      {GREEN}{line}{RESET}", end="")
        elif line.startswith("-") and not line.startswith("---"):
            print(f"      {RED}{line}{RESET}", end="")
        else:
            print(f"      {DIM}{line}{RESET}", end="")


def run_tests(filter_suite=None):
    suites = get_suites(filter_suite)
    total_passed = total_failed = total_errors = total_skipped = 0

    for suite in suites:
        executable = SUITE_EXECUTABLES.get(suite)
        tests = get_tests(suite)

        print(f"\n{BOLD}[{suite.upper()}]{RESET}  {DIM}{executable}{RESET}")
        print("─" * 50)

        if not executable or not executable.exists():
            print(f"  {YELLOW}[SKIP]{RESET}  executable not found: {executable}")
            total_skipped += len(tests)
            continue

        passed = failed = errors = skipped = 0

        input_filename = SUITE_INPUT_FILES.get(suite, "test.asm")
        for test_dir in tests:
            name = test_dir.name
            status, detail, diff = run_single_test(executable, test_dir, input_filename)

            if status == "pass":
                print(f"  {GREEN}[PASS]{RESET}  {name}")
                passed += 1
            elif status == "fail":
                print(f"  {RED}[FAIL]{RESET}  {name}")
                print_diff(diff)
                failed += 1
            elif status == "error":
                print(f"  {RED}[ERROR]{RESET} {name}")
                print(f"         {DIM}{detail}{RESET}")
                errors += 1
            else:
                print(f"  {YELLOW}[SKIP]{RESET}  {name}  {DIM}({detail}){RESET}")
                skipped += 1

        total = passed + failed + errors + skipped
        if failed == 0 and errors == 0:
            color = GREEN
        else:
            color = RED
        print(f"\n  {color}{BOLD}{passed} passed{RESET}", end="")
        if failed:   print(f"  {RED}{failed} failed{RESET}", end="")
        if errors:   print(f"  {RED}{errors} errors{RESET}", end="")
        if skipped:  print(f"  {YELLOW}{skipped} skipped{RESET}", end="")
        print(f"  {DIM}/ {total} total{RESET}")

        total_passed  += passed
        total_failed  += failed
        total_errors  += errors
        total_skipped += skipped

    grand_total = total_passed + total_failed + total_errors + total_skipped
    print(f"\n{'=' * 50}")
    color = GREEN if total_failed == 0 and total_errors == 0 else RED
    print(f"{color}{BOLD}  {total_passed} passed  |  {total_failed} failed  |  {total_errors} errors  /  {grand_total} total{RESET}\n")

    return total_failed + total_errors


def update_expected(filter_suite=None):
    suites = get_suites(filter_suite)

    for suite in suites:
        executable = SUITE_EXECUTABLES.get(suite)
        tests = get_tests(suite)

        print(f"\n{BOLD}[{suite.upper()}]{RESET}")
        print("─" * 50)

        if not executable or not executable.exists():
            print(f"  {YELLOW}[SKIP]{RESET}  executable not found")
            continue

        input_filename = SUITE_INPUT_FILES.get(suite, "test.asm")
        for test_dir in tests:
            asm_file      = test_dir / input_filename
            expected_file = test_dir / "expected.txt"

            if not asm_file.exists():
                continue

            result = subprocess.run(
                [str(executable), "-o", str(expected_file), str(asm_file)],
                capture_output=True
            )

            if expected_file.exists() and expected_file.stat().st_size > 0:
                print(f"  {GREEN}[UPDATED]{RESET} {test_dir.name}")
            else:
                print(f"  {RED}[FAILED]{RESET}  {test_dir.name}")
                print(f"           {DIM}{result.stderr.decode().strip()[:200]}{RESET}")


if __name__ == "__main__":
    args = sys.argv[1:]

    suite_filter = None
    for arg in args:
        if not arg.startswith("--"):
            suite_filter = arg

    if "--update" in args:
        update_expected(suite_filter)
    else:
        exit_code = run_tests(suite_filter)
        sys.exit(exit_code)
