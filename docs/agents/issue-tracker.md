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
