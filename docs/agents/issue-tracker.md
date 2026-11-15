# Issue tracker: Beads Rust (`br`)

Issues and specs for this repo live locally in `.beads/` managed by `br` (`beads_rust`). Sync to git is explicit via JSONL.

## Binary & Environment

- Binary: `br` (typically at `~/.local/bin/br`)
- Actor: Always resolve actor at runtime:
  ```bash
  ACTOR="${BR_ACTOR:-assistant}"
  ```
- Always pass `--json` for structured tool consumption.

## Conventions

- **Create an issue**:
  ```bash
  br create --actor "$ACTOR" "<title>" --priority <0-4> --type <task|bug|feature|epic> --description "<desc>"
  ```
- **Read an issue**:
  ```bash
  br show <id> --json
  ```
- **List / Find work**:
  ```bash
  br ready --json                         # Unblocked actionable work
  br list --status open --sort priority --json
  br blocked --json                       # Blocked work
  ```
- **Claim / In-progress**:
  ```bash
  br update --actor "$ACTOR" <id> --status in_progress --claim
  ```
- **Dependencies**:
  ```bash
  br dep add <child> <parent>             # child depends on parent
  br dep cycles --json                    # MUST return empty
  ```
- **Apply / remove labels**:
  ```bash
  br label add <id> <label>
  br label remove <id> <label>
  ```
- **Close**:
  ```bash
  br close --actor "$ACTOR" <id> --reason "<summary of resolution>"
  ```
- **Sync to Git**:
  `br` never runs git commands automatically. Sync is explicit:
  ```bash
  br sync --flush-only
  git add .beads/ && git commit -m "docs(beads): sync issues"
  ```

## When a skill says "publish to the issue tracker"

Create an issue via `br create --actor "$ACTOR" ...`.

## When a skill says "fetch the relevant ticket"

Read it via `br show <id> --json`.

## Triage & Graph Queries (`bv`)

`bv` analyzes `.beads/issues.jsonl` for graph-aware triage (rankings, blockers, execution tracks); `br` owns creating, claiming, modifying, closing, and flushing issues (`br sync --flush-only` refreshes `.beads/issues.jsonl` after mutations).

**CRITICAL: Always pass `--robot-*` flags. Bare `bv` launches an interactive TUI that blocks the session.**

- **Triage summary**: `bv --robot-triage` (ranked recommendations, quick wins, blockers to clear, project health)
- **Top pick**: `bv --robot-next` (single highest-impact item + claim command)
- **Parallel tracks**: `bv --robot-plan` (supports `--label <label>` and `--recipe actionable`)
- **Graph metrics & hygiene**: `bv --robot-insights`, `bv --robot-priority`, `bv --robot-alerts`, `bv --robot-suggest`
- **History & graph export**: `bv --robot-diff --diff-since <ref>`, `bv --robot-graph [--graph-format=json|dot|mermaid]`

Because `bv` reads a snapshot and does not reserve work, verify live issue state with `br show <id> --json` before claiming via `br update --actor "$ACTOR" <id> --status in_progress --claim`.
