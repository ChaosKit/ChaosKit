# Agent Guidelines

## Agent skills

### Issue tracker & triage

Tracked locally in `.beads/` using `br` (Beads Rust) for issue mutations and `bv --robot-*` for graph-aware triage (never run bare `bv`). See `docs/agents/issue-tracker.md`.

### Triage labels

Standard 5-role triage vocabulary (`needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`). See `docs/agents/triage-labels.md`.

### Domain docs

Single-context documentation layout (`GLOSSARY.md` and `docs/adr/` at repo root). See `docs/agents/domain.md`.

## Build, Test & Lint

Targets in `Makefile` source `.envrc` automatically when present (see `.envrc.sample`). Override preset with `PRESET=release`.

- **Configure**: `make configure`
- **Build**: `make build`
- **Test (all, requires GPU/display)**: `make test`
- **Test (CPU unit tests, sandbox-safe)**: `make test-unit`
- **Lint (staged/working tree)**: `make lint`
- **Format**: `make format`
