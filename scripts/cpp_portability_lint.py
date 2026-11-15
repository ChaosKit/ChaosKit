#!/usr/bin/env python3
"""C++ portability linter for ChaosKit.

Deterministic checks catching compiler/stdlib differences between macOS
(AppleClang/libc++) and Linux CI (GCC 13/libstdc++/glibc):
  PORT-CMATH — Unqualified C math/cstdlib calls (prefer std:: qualification)
  PORT-IWYU  — Missing direct C++ stdlib headers via clang-tidy
               misc-include-cleaner (when --compile-commands is available)
"""

from __future__ import annotations

import json
import os
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

CPP_SUFFIXES = (".cpp", ".cc", ".cxx", ".h", ".hh", ".hpp", ".hxx")
HEADER_SUFFIXES = (".h", ".hh", ".hpp", ".hxx")

RE_HUNK = re.compile(r"^@@ -\S+ \+(\d+)(?:,(\d+))? @@")

# Match multi-line raw strings, block comments, line comments, string literals,
# and char literals in one pass so newline counts remain unchanged.
RE_MASK_CPP = re.compile(
    r'R"([^\(\s\\]*)\((?:.|\n)*?\)\1"'
    r"|/\*(?:.|\n)*?\*/"
    r"|//[^\n]*"
    r'|"(?:\\.|[^"\\\n])*"'
    r"|'(?:\\.|[^'\\\n])*'"
)

RE_CMATH_CALL = re.compile(
    r"(?<![:\w.>])\b("
    r"abs|fabs|fabsf|fabsl|modf|modff|fmod|fmodf|sqrt|sqrtf|cbrt|hypot|"
    r"pow|powf|exp|exp2|expm1|log|log2|log10|log1p|sin|cos|tan|asin|acos|"
    r"atan|atan2|sinh|cosh|tanh|asinh|acosh|atanh|floor|floorf|ceil|ceilf|"
    r"trunc|truncf|round|roundf|lround|llround|isnan|isinf|isfinite"
    r")\s*\("
)

RE_FUNC_DECL_PREFIX = re.compile(
    r"\b(?:void|bool|int|float|double|auto|inline|constexpr|static|virtual)\s*$"
)

RE_INCLUDE_LINE = re.compile(r'^\s*#\s*include\s+[<"]([^>"]+)[>"]', re.M)
RE_IWYU_WARNING = re.compile(
    r"^(.+?):(\d+):\d+:\s+warning:\s+(.+?)\s+\[misc-include-cleaner\]$"
)

IWYU_IGNORE_HEADERS = (
    r"ChaosKit/|Qt|\.framework/|gmock|gtest|mapbox|pcg|magic_enum|"
    r"google/protobuf|OpenGL|GL/"
)


@dataclass(frozen=True)
class Finding:
    file: str
    line: int
    rule: str
    message: str

    def __str__(self) -> str:
        return f"{self.file}:{self.line} {self.rule} {self.message}"


def merge_ranges(lines: set[int]) -> list[list[int]]:
    """Collapse a set of 1-based line numbers into merged [start, end] ranges."""
    if not lines:
        return []
    ordered = sorted(lines)
    ranges: list[list[int]] = [[ordered[0], ordered[0]]]
    for ln in ordered[1:]:
        if ln == ranges[-1][1] + 1:
            ranges[-1][1] = ln
        else:
            ranges.append([ln, ln])
    return ranges


def changed_lines(
    diff_args: list[str],
) -> tuple[str, dict[str, set[int]], dict[str, list[list[int]]]]:
    """Return repo root, per-file changed line sets, and merged line ranges."""

    def git(*args: str) -> str:
        return subprocess.run(
            ["git", *args], capture_output=True, text=True, check=True
        ).stdout

    root = git("rev-parse", "--show-toplevel").strip()
    diff = git(
        "-C",
        root,
        "-c",
        "core.quotePath=false",
        "diff",
        "-U0",
        "--no-color",
        "--no-ext-diff",
        "--src-prefix=a/",
        "--dst-prefix=b/",
        *diff_args,
    )
    changed: dict[str, set[int]] = {}
    current: set[int] | None = None
    for line in diff.splitlines():
        if line.startswith("+++ "):
            path = line[4:].split("\t", 1)[0]
            current = None
            if path.startswith("b/") and path.endswith(CPP_SUFFIXES):
                current = changed.setdefault(path[2:], set())
        elif current is not None and line.startswith("@@"):
            match = RE_HUNK.match(line)
            if match:
                start = int(match.group(1))
                count = int(match.group(2)) if match.group(2) is not None else 1
                if count > 0:
                    current.update(range(start, start + count))

    ranges = {
        rel: merge_ranges(lines) for rel, lines in changed.items() if lines
    }
    return root, changed, ranges


def mask_cpp_preserving_lines(text: str) -> str:
    """Replace strings and comments while keeping exact line numbers intact."""

    def _repl(match: re.Match[str]) -> str:
        newlines = match.group(0).count("\n")
        return ("\n" * newlines) if newlines else " "

    return RE_MASK_CPP.sub(_repl, text)


def check_cmath(
    filepath: str, text: str, target_lines: set[int]
) -> list[Finding]:
    """Flag unqualified C math/cstdlib calls on target lines."""
    norm = filepath.replace(os.sep, "/")
    if norm.endswith("ast/helpers.h") or "ast::helpers" in text:
        return []

    masked_lines = mask_cpp_preserving_lines(text).splitlines()
    findings: list[Finding] = []
    for lineno in sorted(target_lines):
        if lineno < 1 or lineno > len(masked_lines):
            continue
        line = masked_lines[lineno - 1]
        for match in RE_CMATH_CALL.finditer(line):
            prefix = line[: match.start()].rstrip()
            if RE_FUNC_DECL_PREFIX.search(prefix):
                continue
            fn = match.group(1)
            findings.append(
                Finding(
                    file=filepath,
                    line=lineno,
                    rule="PORT-CMATH",
                    message=(
                        f"Qualify '{fn}(...)' with std:: "
                        "(and prefer non-suffixed std:: overloads) "
                        "for glibc/GCC portability"
                    ),
                )
            )
    return findings


def load_compile_units(compile_commands_path: str) -> list[str]:
    """Load non-generated, non-third-party TU paths from compile_commands.json."""
    with open(compile_commands_path, encoding="utf-8") as fh:
        entries = json.load(fh)
    tus: list[str] = []
    seen: set[str] = set()
    for entry in entries:
        real = os.path.realpath(entry["file"])
        norm = real.replace(os.sep, "/")
        if "/_deps/" in norm or "/autogen/" in norm or norm.endswith(".pb.cc"):
            continue
        if real not in seen and os.path.isfile(real):
            seen.add(real)
            tus.append(real)
    return tus


def read_includes(path: str, cache: dict[str, set[str]]) -> set[str]:
    """Return included header strings for a C++ file (cached)."""
    if path not in cache:
        try:
            text = Path(path).read_text(encoding="utf-8", errors="replace")
        except OSError:
            cache[path] = set()
        else:
            cache[path] = set(RE_INCLUDE_LINE.findall(text))
    return cache[path]


def _includes_target(includes: set[str], target_name: str) -> bool:
    return any(
        inc == target_name or inc.endswith("/" + target_name) for inc in includes
    )


def resolve_host_tu(
    header_real: str, tus: list[str], tu_set: set[str], inc_cache: dict[str, set[str]]
) -> str | None:
    """Find a compiled .cpp TU that directly or transitively includes header_real."""
    header_dir = os.path.dirname(header_real)
    stem = Path(header_real).stem
    for suffix in (".cpp", "Test.cpp", "_test.cpp"):
        candidate = os.path.realpath(os.path.join(header_dir, stem + suffix))
        if candidate in tu_set:
            return candidate

    # Prefer TUs in the same directory first, then anywhere in compile_commands.
    ordered_tus = sorted(
        tus, key=lambda p: (os.path.dirname(p) != header_dir, p)
    )
    basename = os.path.basename(header_real)
    for tu in ordered_tus:
        if _includes_target(read_includes(tu, inc_cache), basename):
            return tu

    # Fall back to one-hop umbrella headers in the same directory.
    try:
        siblings = sorted(os.listdir(header_dir))
    except OSError:
        return None
    umbrellas = [
        os.path.basename(sib)
        for sib in siblings
        if sib.endswith(HEADER_SUFFIXES)
        and _includes_target(
            read_includes(os.path.join(header_dir, sib), inc_cache), basename
        )
    ]
    for umb in umbrellas:
        for tu in ordered_tus:
            if _includes_target(read_includes(tu, inc_cache), umb):
                return tu
    return None


def check_iwyu(
    compile_commands_path: str | None,
    root: str,
    target_files: list[str],
    lines_by_file: dict[str, set[int]],
    ranges_by_file: dict[str, list[list[int]]],
) -> list[Finding]:
    """Run clang-tidy misc-include-cleaner when compile_commands and clang-tidy exist."""
    if not compile_commands_path or not os.path.isfile(compile_commands_path):
        return []
    clang_tidy = shutil.which("clang-tidy")
    if not clang_tidy:
        return []

    tus = load_compile_units(compile_commands_path)
    tu_set = set(tus)
    inc_cache: dict[str, set[str]] = {}

    tus_to_run: set[str] = set()
    line_filter: list[dict[str, object]] = []
    header_regexes: list[str] = []
    target_by_real: dict[str, str] = {}

    for filepath in target_files:
        ranges = ranges_by_file.get(filepath)
        if not ranges:
            continue
        real = os.path.realpath(filepath)
        is_header = filepath.endswith(HEADER_SUFFIXES)
        if real in tu_set:
            tus_to_run.add(real)
        elif is_header:
            host = resolve_host_tu(real, tus, tu_set, inc_cache)
            if not host:
                continue
            tus_to_run.add(host)
            rel = os.path.relpath(filepath, root).replace(os.sep, "/")
            header_regexes.append(re.escape(rel) + "$")
        else:
            continue

        target_by_real[real] = filepath
        line_filter.append({"name": real, "lines": ranges})

    if not tus_to_run:
        return []

    build_dir = os.path.dirname(os.path.abspath(compile_commands_path))
    config = json.dumps(
        {
            "CheckOptions": [
                {
                    "key": "misc-include-cleaner.IgnoreHeaders",
                    "value": IWYU_IGNORE_HEADERS,
                }
            ]
        }
    )
    cmd = [
        clang_tidy,
        "-p",
        build_dir,
        "--quiet",
        "--checks=-*,misc-include-cleaner",
        f"--line-filter={json.dumps(line_filter)}",
    ]
    if header_regexes:
        cmd.append(f"--header-filter={'|'.join(header_regexes)}")
    cmd.extend([f"--config={config}", *sorted(tus_to_run)])

    proc = subprocess.run(cmd, capture_output=True, text=True)
    findings: list[Finding] = []
    seen: set[tuple[str, int, str]] = set()
    for line in proc.stdout.splitlines():
        match = RE_IWYU_WARNING.match(line)
        if not match:
            continue
        diag_real = os.path.realpath(match.group(1))
        filepath = target_by_real.get(diag_real)
        if not filepath:
            continue
        lineno = int(match.group(2))
        if lineno not in lines_by_file.get(filepath, ()):
            continue
        msg = match.group(3)
        key = (filepath, lineno, msg)
        if key not in seen:
            seen.add(key)
            findings.append(
                Finding(file=filepath, line=lineno, rule="PORT-IWYU", message=msg)
            )
    return findings


def print_summary(findings: list[Finding], root: str) -> None:
    """Print findings grouped by (rule, message), matching qt_review_lint.py."""
    groups: dict[tuple[str, str], dict[str, list[int]]] = {}
    for f in findings:
        rel = os.path.relpath(os.path.abspath(f.file), root)
        groups.setdefault((f.rule, f.message), {}).setdefault(rel, []).append(
            f.line
        )
    print(f"{len(findings)} finding(s), {len(groups)} distinct rule/message pair(s)")
    for (rule, message), by_file in sorted(groups.items()):
        count = sum(len(v) for v in by_file.values())
        print(f"{rule} {message} ({count})")
        locs = [
            f"{path}:{','.join(str(n) for n in sorted(lines))}"
            for path, lines in sorted(by_file.items())
        ]
        print("  " + "  ".join(locs))


def main() -> int:
    summary = False
    diff_args: list[str] | None = None
    compile_commands: str | None = None
    files: list[str] = []

    for arg in sys.argv[1:]:
        if arg == "--summary":
            summary = True
        elif arg.startswith("--diff="):
            diff_args = shlex.split(arg[len("--diff=") :])
        elif arg.startswith("--compile-commands="):
            compile_commands = arg[len("--compile-commands=") :]
        elif arg in ("-h", "--help"):
            print(
                "Usage: cpp_portability_lint.py [--summary] "
                "[--compile-commands=PATH] (--diff=ARGS | <file...>)"
            )
            return 0
        else:
            files.append(arg)

    root: str | None = None
    lines_by_file: dict[str, set[int]] = {}
    ranges_by_file: dict[str, list[list[int]]] = {}

    if diff_args is not None:
        if files:
            print("Error: --diff takes no file arguments", file=sys.stderr)
            return 2
        try:
            root, rel_changed, rel_ranges = changed_lines(diff_args)
        except (OSError, subprocess.CalledProcessError) as exc:
            detail = getattr(exc, "stderr", None) or str(exc)
            print(f"Error: git diff failed: {detail.strip()}", file=sys.stderr)
            return 2
        for rel in sorted(rel_changed):
            full = os.path.join(root, rel)
            if os.path.isfile(full):
                files.append(full)
                lines_by_file[full] = rel_changed[rel]
                if rel in rel_ranges:
                    ranges_by_file[full] = rel_ranges[rel]
        if not files:
            print(f"No changed C++ files in git diff {' '.join(diff_args)}")
            return 0
    elif not files:
        print("Error: no input files specified", file=sys.stderr)
        return 2
    else:
        abs_files = [os.path.abspath(f) for f in files]
        files = abs_files
        dirs = [os.path.dirname(f) for f in files]
        root = os.path.commonpath(dirs)
        try:
            top = subprocess.run(
                ["git", "rev-parse", "--show-toplevel"],
                capture_output=True,
                text=True,
                check=True,
            ).stdout.strip()
            if top:
                root = top
        except (OSError, subprocess.CalledProcessError):
            pass

    all_findings: list[Finding] = []
    for filepath in files:
        try:
            text = Path(filepath).read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            print(f"Error reading {filepath}: {exc}", file=sys.stderr)
            return 2
        if diff_args is None:
            line_count = len(text.splitlines())
            target = set(range(1, line_count + 1))
            lines_by_file[filepath] = target
            if line_count > 0:
                ranges_by_file[filepath] = [[1, line_count]]
        else:
            target = lines_by_file[filepath]
        all_findings.extend(check_cmath(filepath, text, target))

    assert root is not None
    all_findings.extend(
        check_iwyu(
            compile_commands, root, files, lines_by_file, ranges_by_file
        )
    )
    all_findings.sort(key=lambda f: (f.file, f.line, f.rule))

    if summary:
        print(f"Portability check (root {root}/):")
        print_summary(all_findings, root)
    else:
        for finding in all_findings:
            print(finding)

    return 1 if all_findings else 0


if __name__ == "__main__":
    sys.exit(main())
