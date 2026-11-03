---
compatibility: Designed for Claude Code, GitHub Copilot, and similar agents.
description: Invoke when the user asks to review, check, audit, or look over Qt6 QML code -- or suggest before committing. Runs deterministic linting (48+ rules) then deep-analysis agents (scaled to the size of the change) covering bindings, layout, loaders, delegates, states, and performance. Optionally invokes system qmllint for type-level checks. Reports only high-confidence issues (>80/100) with structured mitigations. Read-only -- never modifies code.
disable-model-invocation: false
license: LicenseRef-Qt-Commercial OR BSD-3-Clause
metadata:
    author: qt-ai-skills
    category: review
    github-path: skills/qt-qml-review
    github-ref: refs/heads/main
    github-repo: https://github.com/TheQtCompanyRnD/agent-skills
    github-tree-sha: e5ea0bbbd68810e32db69f538f1adf6341736450
    qt-version: 6.x
    version: "2.0"
name: qt-qml-review
---
# Qt QML Code Review

Read-only review of Qt6 QML code: a deterministic Python linter (with
qmllint folded in when available) first, then agent-driven deep
analysis across seven rule domains, then one consolidated report.
Suggest it before the user commits QML code.

## Scope detection

**Diff scope** -- "this commit", "these changes", "the diff", "what
I changed", "staged changes", "before I commit". The range is `HEAD`
(working tree plus index), `--cached` for staged changes only, or
`HEAD~1..HEAD` for "this commit". Do not load the diff itself: the
linter's `--diff` mode (Phase 1) lists the changed files with their
changed-line counts and reports only findings on changed lines.

**Codebase scope** -- "review the codebase", "audit src/", or an
explicit path with no commit language. Glob `*.qml` under the path.

## Token budget rules

These rules exist to keep the review cheap. Follow them.

1. **Do not load the reference files into the main context.** Agents
   read `references/agent-rules.md` themselves; you pass its absolute
   path and the section IDs. Only an inline run (Phase 2) reads it
   in the main context. Lint messages are self-describing; the
   checklist is for humans.
2. **Pass lint results in `--summary` form** (grouped by rule, paths
   relative to a root printed once), never the raw per-line listing
   or qmllint's JSON.
3. **Point agents at content, never paste it.** Every token you write
   into a prompt is output on the orchestrating model, the most
   expensive tokens in the review. Prompts carry paths, commands and
   section IDs; the agents load the rules, the files and the diff.
   In diff scope an agent gets the `git diff -U50 <range> -- <files>`
   command instead of the hunks, and reads other files only to trace
   a specific symbol.
4. **Scale the agent count to the scope** (see Phase 2). Launch no
   agents for a small scope and split a large one into shards of about
   equal line count.
5. **Use the dedicated review agent where the tool has one.** If the
   `qt-development-skills:qml-review-agent` definition shipped with
   this plugin is available, launch the agents with it (read-only,
   tools limited to Read, Grep, Glob and read-only Bash); otherwise
   use the tool's general-purpose agent.

## Execution order

Never skip a phase.

### Phase 1: Linting (Python linter + qmllint)

```bash
# codebase scope
python3 <skill>/references/lint-scripts/qt_qml_lint.py --summary --qmllint <files...>
# diff scope, run inside the repository; no file arguments
python3 <skill>/references/lint-scripts/qt_qml_lint.py --summary --qmllint --diff=<range>
# fall back to `python` if python3 is not found
```

Python 3.7+, no dependencies. If Python is unavailable, warn and
continue (in diff scope, take the file list from
`git diff --stat <range>`). The linter is authoritative; do not
second-guess its output. Rule families: IMP, ORD, BND, LAY, LDR, DEL,
STA, IMG, PRF, STY, SIG, ERR, JS (48+ checks). Exit code 1 simply
means findings exist; 2 means a usage or git error.

`--qmllint` also runs qmllint and merges its warnings as `QL:<id>`
rows; add `--qmllint-args="-I <dir>"` for a build directory with QML
modules. The linter looks in `$QT_HOST_PATH/bin`, on PATH, in the
host bin directory reported by `qtpaths` or `qmake`, and in the
newest Qt 6 kit of a Qt online installer under `~/Qt`, `C:\Qt` or
`/opt/Qt` (`<version>/<kit>/bin`, kits such as `gcc_64`, `macos`,
`msvc2022_64`, `mingw_64`). If it reports `not available` but Qt is
installed elsewhere (a custom installer path, or `CMAKE_PREFIX_PATH`
in the project's `CMakeCache.txt`), rerun once with
`--qmllint=<that kit>/bin`; it accepts the binary or its bin
directory. Do not search further. Copy the printed `qmllint:` status
into the report header. qmllint is authoritative for
type-level issues, the Python linter for style, ordering and
performance patterns.

### Phase 2: Deep analysis (inline, or one agent per shard)

The seven sections of `references/agent-rules.md` are the analysis.
Agents read it themselves; read it yourself only when running inline:

| Section | Mission |
|---|---|
| A1 | Bindings & Properties |
| A2 | Layout & Anchoring |
| A3 | Component Loading & Lifecycle |
| A4 | ListView & Delegates |
| A5 | States, Transitions & Migration |
| A6 | Performance & Code Quality |
| A7 | Logic & Wiring |

Choose the fan-out from the scope size. Count the lines of the files
in scope (the changed lines in diff scope), taken from the lint
summary or `wc -l`. A block is about 1,500 lines; the line count
alone decides the split, never the number of files.

| Scope | Agents |
|---|---|
| Up to 1,500 lines | 0 (inline) |
| More than 1,500 lines | one per shard, `ceil(lines / 1500)` shards |

**Inline.** A small scope does not repay the cost of agents, which
each rebuild the context and roughly double the review's token cost
for the same findings. Do the analysis yourself: read
`references/agent-rules.md` and the files (or the diff) with the one
Bash call shown under the agent prompt below, and apply A1-A7 in a
single pass under the same rules, confidence thresholds and output
format. Do not re-report lint rows, and do not run inline above the
limit because the code looks simple. Report `Agents: 0 (inline)`.

**Shards.** Cut the files, in directory order, into
n = `ceil(lines / 1500)` shards of about `lines / n` lines each; how
many files a shard holds does not matter. Keep a directory in one
shard where it fits, so that components and the files that
instantiate them land together; split a larger directory by file. A
file larger than a shard gets a shard to itself. Each shard agent applies all seven sections to its shard only and may Grep
the whole scope to trace an id or a model role. Run up to six shard
agents at a time and the rest in a second wave. Cross-shard checks
(ids, model roles) are then best effort; say so in the report
header.

Launch the agents in parallel, named after their shard
(e.g. "Review shard 2/6: qml/controls"). Keep each prompt short; it contains
only:

1. The absolute path of `references/agent-rules.md` and the section
   IDs to apply (A1-A7)
2. The root directory from the lint summary, the files in scope
   relative to it and, in diff scope, the
   `git diff -U50 <range> -- <files>` command for them
3. The lint summary rows (including `QL:` rows) for that agent's own
   files only (a shard agent never needs the other shards' rows),
   with the instruction "do not re-report these lines"
4. The instruction to load everything with one Bash call before
   analysing, and to return findings in the format at the top of the
   rules file. Each tool call re-sends the agent's whole context, so
   one batched read instead of one Read per file is the single
   largest saving on large scopes. Both forms print per-file line
   numbers for the `file:line` locations (`cat -n` on several files
   numbers them as one run, and `git diff` has no line numbers):

   ```bash
   # codebase scope
   cd <root> && cat <rules> && for f in <files...>; do echo "== $f"; cat -n "$f"; done
   # diff scope; numbers context and added lines with the new-file line
   cd <root> && cat <rules> && git diff -U50 <range> -- <files> | awk '/^@@/{split($3,a,/[+,]/);n=a[2];h=1;print;next} /^diff /{h=0} h&&/^[ +]/{printf "%5d %s\n",n++,$0;next} {print}'
   ```

Agents are read-only and must never edit files. In tools without
agents, run each mission as whatever prompt chain or subprocess the
tool supports; the contract is the same.

### Phase 3: Consolidation

Merge lint and agent findings. Deduplicate on file+line+issue; when
duplicates disagree on confidence, keep the block with the highest
score and its trace, never average scores. Keep at most 10
investigation targets, and renumber the agents' D-/I- blocks
consecutively. Findings below confidence 60 are suppressed.
Write the report in the format below.

## Output format

Use exactly this structure. Lint and qmllint findings are grouped by
rule with one mitigation each; only deep-analysis findings get a full
block, copied verbatim from the agents.

```
## QML Code Review Report

**Scope**: [diff: `git diff HEAD~1..HEAD` | files: <paths>]
**Files reviewed**: N
**Issues found**: N (M from lint, K from deep analysis)
**Agents**: N (shards ... | inline)
**qmllint**: [ran / not available / failed]

---

### Lint findings

| Rule | Count | Locations | Mitigation |
|---|---|---|---|
| IMP-2 Versioned import | 3 | `Main.qml:1,2`, `Panel.qml:1` | Drop the version numbers. |
| ... | | | |

---

### Deep analysis findings

<[D-NNN] blocks from the agents>

---

### Investigation targets (human verification needed)

<[I-NNN] blocks from the agents, at most 10>

---

### Summary

| Category | Lint | Deep | Investigate | Total |
|---|---|---|---|---|
| ... | N | N | N | N |
| **Total** | **M** | **K** | **I** | **N** |

Findings below confidence 60 are suppressed.
```

## References

- `references/agent-rules.md` -- agent missions A1-A7 and the agent
  output format (read by the agents, not the orchestrator)
- `references/qt-qml-review-checklist.md` -- rationale for every
  linter rule (human reference, not loaded during a review)
- `references/lint-scripts/qt_qml_lint.py` -- single-pass linter,
  `--summary` for grouped output, `--diff=<range>` for changed lines
  only, `--qmllint` to merge qmllint, `--json` for tooling

---

Copyright (C) 2026 The Qt Company.
