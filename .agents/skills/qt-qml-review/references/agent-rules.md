# Qt6 QML Review -- Deep-Analysis Agent Rules

Semantic checks that need symbol tracing or cross-file reasoning and
therefore cannot be expressed as linter regexes. Grouped by review
mission. Each agent loads this file itself, in the same `cat` as its
QML files, and applies **only the section(s) named in its prompt**.
Agents must not load the lint checklist.

Anything the linter or qmllint already reports is deliberately absent
here. Agents receive the lint summary rows for their files and must
not re-report those lines.

## Confidence and output format

Score every candidate 0-100. Report 80 and above as findings, 60-79
as investigation targets (the orchestrator keeps at most 10 in total),
and drop anything below 60.

- **80-100**: the defective code path is visible in the loaded source,
  or confirmed by a Grep anywhere under the root. Documented Qt Quick
  behaviour counts as confirmed. Running the code is never required,
  and "this is example code" or "the case may never occur" does not
  lower the score.
- **60-79**: whether it is a defect depends on something you cannot
  see: C++ types or context properties outside the root, the
  component's users, model contents at runtime, or intent.
- **Below 60**: a suspicion with no supporting code path.

Prefer a few confirmed findings over many guesses. Return the blocks
below verbatim so the orchestrator can merge them without rewriting;
use the mission name of your section as the category. If you found
nothing, say so in one line.

```
#### [D-NNN] <Short title>
- **File**: `path/to/file.qml:42`
- **Category**: <mission name>
- **Confidence**: NN/100
- **Finding**: <what is wrong>
- **Trace**: <ids, roles or types followed, what was checked>
- **Mitigation**: <what to do, in prose -- no code patches>

#### [I-NNN] <Short title>
- **File**: `path/to/file.qml:42`
- **Category**: <mission name>
- **Confidence**: NN/100
- **Finding**: <what is suspected>
- **Unverified because**: <what could not be confirmed>
- **How to verify**: <specific action for the reviewer>
```

---

## A1. Bindings & Properties

- **Multi-cycle binding loops.** The runtime only detects single-cycle
  loops (`QML: Binding loop detected`). A handler on A writes B while
  B's binding reads A is silent and degrades performance. Typical
  source: `implicitWidth`/`implicitHeight` bound inside layouts.
- **Alias chains.** Alias-to-alias is fragile: the value is
  `undefined` until every intermediate component has finished
  initialisation, and aliases are inactive during construction.
- **Unqualified lookup.** Bare `someProperty` instead of
  `root.someProperty` resolves through the dynamic scope chain and
  blocks `qmlsc`. Complements qmllint's `unqualified` warning with
  the semantic reason.
- **`Qt.binding()` closures capturing loop variables** declared with
  `var` (function scope) -- every closure sees the last value.
- **Missing `pragma ComponentBehavior: Bound`** in files whose
  delegates or inline components reach outer-scope ids. With the
  pragma, data must flow via `required property`. Qt intends to make
  `Bound` the default.
- **Missing `readonly`** on properties that are bound once and never
  imperatively assigned.

## A2. Layout & Anchoring

- **Anchoring to an invisible item.** Resolve the anchor target id
  and check its `visible` property; an invisible anchor target still
  positions the item but usually signals a logic error.
- **Cross-branch anchoring.** Anchors must reference `parent` or a
  sibling. The linter only catches literal `parent.parent`; trace
  resolved ids for other cases.
- **Implicit-size feedback in Layouts.** Children binding
  `implicitWidth`/`implicitHeight` to layout-derived values create
  size-negotiation loops.
- **Missing `Layout.fillWidth`/`Layout.fillHeight`** on items that are
  evidently meant to stretch (e.g. a lone `TextField` in a `RowLayout`).
- **Nested Layouts with no sizing policy** -- ambiguous negotiation
  between parent and child layouts.

## A3. Component Loading & Lifecycle

- **`Component.createObject()` without lifecycle management.** The
  return value must be kept in a variable or the object must have a
  parent; otherwise it is garbage-collected at an arbitrary time or,
  if parented and never destroyed, leaks.
- **Loader switching between `source` and `sourceComponent` at
  runtime** (from states, bindings or JS). Unsupported; the linter
  (LDR-5) only catches both set in one block.
- **Image with dynamic or network `source` and no `status` handling.**
  Loads can fail; check `Image.status === Image.Error`.
- **`Connections` whose `target` changes at runtime** and does not
  tolerate `null` (explicit `target: null` is fine; a binding that can
  evaluate to `undefined` is not).
- **Context properties** (`rootContext()->setContextProperty()`) in
  the accompanying C++: re-evaluated on every access, globally scoped,
  invisible to tooling, prevents compilation. Prefer `QML_ELEMENT`,
  singletons, or `createWithInitialProperties`.
- **Ownership at the QML/C++ boundary.** The engine takes ownership
  of `QObject`s returned from `Q_INVOKABLE` methods but not from
  properties. Parentless objects returned from invokables without an
  explicit `QJSEngine::setObjectOwnership` are a use-after-free or
  leak risk. Keep ownership by parenting or `CppOwnership`; hand it
  over with `JavaScriptOwnership`.
- **Non-const range-for over Qt containers** in the C++ model or
  backend (e.g. a `QList` member in `data()`) detaches and deep-copies
  the implicitly shared data; use `std::as_const` or a const ref.

## A4. ListView & Delegates

- **Missing `required property int index`** (or `modelData`) in a
  delegate that declares other required properties and uses `index`:
  built-in roles are not injected once any required property exists.
  Resolve the delegate from the view's `delegate:` assignment.
- **Role access not backed by the model.** `model.roleName` or a
  `required property` for a role the C++ model's `roleNames()` (or
  the `ListModel` elements) never defines.
- **Delegate complexity.** Nested Repeaters, several Loaders or heavy
  bindings multiply cost by item count and stall scrolling.
- **`currentIndex` reliability.** Defaults to 0, not -1, once a model
  is set. Known bugs QTBUG-48633 (model change resets to 0) and
  QTBUG-93293 (initial binding ignored); workaround is re-applying in
  `onModelChanged`.
- **`DelegateChooser` on a non-`QAbstractItemModel`.** The choice is
  made once at creation and never re-evaluated.
- **Pooled delegates staying visible.** With `reuseItems: true`, a
  delegate that hides or shows itself must reset that state in
  `ListView.onReused` / `onPooled`.

## A5. States, Transitions & Migration

- **`restoreEntryValues` surprises.** `PropertyChanges` defaults to
  `restoreEntryValues: true`, so values written imperatively while in
  a state revert on exit. Flag code that expects them to persist.
- **`Binding.restoreMode` default change.** Qt 5 `RestoreNone` became
  Qt 6 `RestoreBindingOrValue`. Migrated code relying on a `Binding`
  "sticking" after `when` turns false silently reverts.
- **Component-level `states` in a reusable type** that the linter
  did not flag (STA-3 only fires when `required property` is present).
  `states` is a list property; external assignment appends rather than
  replaces. Wrap in `StateGroup`.
- **Transitions that will misfire when states are added.** A
  catch-all transition is only selected when no `from`/`to` rule
  matches; verify the intent when several transitions coexist.

## A6. Performance & Code Quality

- **Expensive expressions in hot bindings.** Function calls that
  re-run on each dependency change; cache in a `readonly property`.
- **`QRegularExpression` or heavy computation inside loops** --
  compile or compute once outside the loop.
- **Default `textFormat` where `Text.PlainText` would do.** The
  linter flags explicit `RichText`; the agent flags `AutoText`
  defaults on text that is never formatted (parsing overhead).
- **`font.preferShaping: false` opportunity** on Text in delegates or
  frequently updated labels that never need ligatures or complex
  scripts.
- **Direction of communication.** Signals go up (child notifies
  owner); functions go down (owner instructs child). Handlers must
  not mutate the emitter. Never emit C++ signals from QML.
- **Unreferenced `id`s.** Only assign an id that something uses;
  unused ids add duplicate-id risk.
- **Scattered internal properties.** Consolidate private state into
  `QtObject { id: privates }` rather than sprinkling properties over
  visual items.
- **Singletons holding instance data.** Prefer property injection for
  testability.
- **`MouseArea` where a pointer handler fits.** `TapHandler`,
  `DragHandler`, `HoverHandler` are composable and multi-touch safe;
  `MouseArea` grabs exclusively and conflicts when mixed.
- **Reusable component with explicit `width`/`height`.** Provide
  `implicitWidth`/`implicitHeight` from content instead so consumers
  can resize.
- **`parent` used unguarded** in delegates (it is the view's
  container, not the view; use `ListView.view` or an id), Loader items
  (it is the Loader), or Popups (it is the overlay). `parent` can also be `null` during
  creation and destruction.
- **Heavy JavaScript blocks.** Large imperative JS forces interpreter
  fallback and blocks `qmlsc`; move logic to C++ or bindings.

## A7. Logic & Wiring

Defects where the code is valid but does not do what the UI promises.
Each needs a trace: Grep the whole scope for every definition you
claim is unused, and read both sides of every pair you compare. A
check whose other half lies outside the root (C++ callers, backend
ranges) is an investigation target, not a finding.

- **Declared but never wired.** A setting, property, function, signal
  or handler that is written or declared but never read or called
  anywhere under the root; the control or pipeline it belongs to is a
  no-op.
- **Results ignored.** A validation, error or lookup result that is
  computed and then not used where the accept, save or commit follows.
- **Expressions that cannot vary.** A comparison of a value with
  itself, a fallback equal to the primary, a condition on a constant;
  check what the expression resolves to, not how it reads.
- **Misses not handled.** A function that returns an empty value
  (`{}`, `null`, `undefined`, `-1`) when nothing is found, with a
  caller that uses the result before testing it.
- **Two sides that disagree.** Paired code where one half does what the
  other skips: two handlers for one user action, save without load, a
  control range narrower than the model range, a change handler that
  reads the new value where it needs the old one.

---

Copyright (C) 2026 The Qt Company.
