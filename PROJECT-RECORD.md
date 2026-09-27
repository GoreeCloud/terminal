# GoreeCloud Terminal — Project Record

> **Repository:** `GoreeCloud/terminal`  
> **Project:** GoreeCloud Terminal  
> **Document:** `PROJECT-RECORD.md`  
> **Authority:** Canonical repository-local significant project history  
> **Last updated:** 2026-09-27  
> **Owner:** GoreeCloud

## Record Purpose

This file preserves significant historical, governance, architecture, migration, release, security, privacy, and validation events for GoreeCloud Terminal.

Routine feature requirements belong in [PROJECT-SPECIFICATIONS.md](./PROJECT-SPECIFICATIONS.md). Routine release changes should be recorded in the repository changelog when one exists.

## 2026-09-27 — Initial Repository Specification Established

### Event

The initial canonical repository-local product specification for GoreeCloud Terminal was established in `PROJECT-SPECIFICATIONS.md`.

The specification defines GoreeCloud Terminal as a modern Linux terminal and command-line administration environment for local administration, remote administration, development, containers, troubleshooting, recovery, and long-running infrastructure operations.

### Source Material

The owner supplied the initial product definition and required capability set on 2026-09-27. The specification organizes that material into repository-local normative requirements while preserving a strict distinction between intended capabilities and verified implementation state.

### Repository State Verified Before Documentation

Live GitHub verification on 2026-09-27 established:

- repository: `GoreeCloud/terminal`;
- owner: `GoreeCloud`;
- visibility: public;
- default branch: `main`;
- repository was not archived;
- the connected GoreeCloud identity had administrative and push permission; and
- neither mandatory project record existed before this documentation pass.

No implementation footprint was used as evidence that the requested features were already built.

### Documentation Authority

GoreeCloud project governance requires the repository-local files:

- `PROJECT-SPECIFICATIONS.md`; and
- `PROJECT-RECORD.md`.

These files are the project specification and significant-history authority for this repository. A parallel Google Drive project specification was intentionally not created.

### Initial Specification Commit

The initial `PROJECT-SPECIFICATIONS.md` was committed to `main` as:

`378c948290316d54d6feed670ed755ab6ba5f9a0`

### Verification Boundary

This event records the establishment of the specification, not completion of the implementation.

The following remain separate evidence questions for future project work:

- which required capabilities are implemented;
- which capabilities have automated test coverage;
- which capabilities have representative Linux runtime evidence;
- which terminal protocols are supported;
- which remote, shell, container, and session-management mechanisms are implemented;
- which performance characteristics have been measured;
- which accessibility requirements have been validated;
- which security and privacy controls have been verified;
- current lifecycle state;
- release readiness; and
- production or Stable/Anchor qualification.

Future project-record entries must cite concrete repository, pull-request, commit, workflow, release, or runtime evidence where applicable.

## 2026-09-27 — Native GTK4/VTE Development Foundation Integrated

### Event

The first bounded native implementation foundation for GoreeCloud Terminal was integrated into authoritative `main` through pull request #1.

The implementation establishes a native GTK 4 application shell with GNOME VTE as a bounded terminal-emulation and PTY dependency. GoreeCloud retains product authority over application behavior, session/workspace design, execution context, safety behavior, profiles, remote administration, configuration, automation, platform integration, and visual direction.

### Exact Source Evidence

- authoritative base before the change: `347a70d5916c072017d9623e98bd374b6bc45bee`;
- exact merge candidate: `21356d7d20f8482a4879f85e00786dc8995de447`;
- pull request: #1, **Native GTK4 terminal foundation**;
- merge method: squash;
- authoritative merged commit: `0e743e775dd7d770ec979a88eb39d946483f1e79`;
- merged tree: `38891f9522b8b0df913bfeddcbf095f8a8cb0466`; and
- GitHub reports the merged commit signature as verified.

### Validation Evidence

The exact candidate passed both push and pull-request validation before merge:

- Native Foundation run `36350340491` — push — success;
- Native Foundation run `36350466598` — pull request — success.

The validated workflow completed:

- repository-governance validation;
- Ubuntu 24.04 dependency installation;
- Meson configuration;
- native compilation;
- GLib context-helper tests; and
- install-layout verification.

After merge, authoritative `main` commit `0e743e775dd7d770ec979a88eb39d946483f1e79` passed Native Foundation run `36350564392`.

### Implemented Development Boundary

Verified source now includes:

- GTK 4 application/window lifecycle;
- VTE GTK4 terminal widget and asynchronous PTY-backed local shell startup;
- `$SHELL` selection with a `/bin/sh` fallback;
- 10,000-line default scrollback;
- copy/paste actions and `Ctrl+Shift+C` / `Ctrl+Shift+V`;
- local-versus-elevated-local context presentation;
- current `user@host`, working-directory, and session-state presentation;
- terminal-title propagation;
- Meson/Ninja build and install plumbing;
- focused context-helper tests;
- repository-governance validation;
- Platform Contract 0.4 declaration covering all nine Integral Platform Systems; and
- a bounded Glaze-inspired Development presentation candidate.

### Security and Privacy Review Boundary

The integrated foundation contains no remote protocol implementation, reusable credentials, telemetry, analytics, hidden network reporting, or automatic privilege escalation.

The pull-request diff review found no embedded secret material. Remote administration, credential/key handling, persistence, synchronization, diagnostics, and platform-system integrations remain separately gated future work.

### Repository Governance Boundary

Live GitHub verification at integration time showed `main` was not protected and no repository ruleset was present. The change nevertheless used an isolated branch, exact-head CI, a pull request, expected-head guarded squash merge, authoritative-branch readback, and post-merge CI.

Repository-level branch/ruleset protection remains a separate GoreeCloud governance obligation and is not represented as remediated by this source integration.

### Lifecycle and Acceptance Boundary

GoreeCloud Terminal remains **Development**.

This integration does not establish:

- tabs or split panes;
- SSH or remote administration;
- profiles or configuration persistence;
- detachable or reconnectable sessions;
- container workflows;
- terminal-protocol completeness;
- GPU acceleration or performance targets;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or deployment;
- release qualification; or
- Stable/Anchor lifecycle status.

Those capabilities remain subject to the applicable planned-feature, validation, release, and production-acceptance gates.

## 2026-09-27 — Session/Workspace, Multi-Tab, and Onboarding Slice Integrated

### Event

Pull request #4 integrated the next bounded Development slice: explicit terminal-session/workspace modules, multiple local tabs, working-directory-aware tab creation, and first-run onboarding with durable progress and Help replay.

### Exact Source Evidence

- authoritative base: `89ea64c8d536f5698a9e56c2362831f7a3bfd3f3`;
- exact merge candidate: `7f93d9207369e72df2c4e7ef71deace0f2d894c7`;
- pull request: #4, **Add session workspace, tabs, and onboarding**;
- merge method: squash;
- authoritative merged commit: `575801a8ebe34bf3f22d5868771ad9283d1e2896`;
- merged tree: `e2e8db43be300b7fec63e6b1a328aec0a612cb19`; and
- GitHub reports the merged commit signature as verified.

### Validation Evidence

The exact candidate passed:

- Native Foundation run `36351979966` — push — success;
- Native Foundation run `36351982244` — pull request — success.

The merged authoritative revision then passed:

- Native Foundation run `36352065673` — push on `main` — success.

The workflow verified repository governance, Meson configuration, native compilation, context tests, onboarding-state tests, and install layout on Ubuntu 24.04.

### Implemented Development Boundary

Verified source now adds:

- explicit `GcTerminalSession` and `GcWorkspace` modules;
- multiple reorderable local shell tabs;
- active-session context and window-title synchronization;
- working-directory-aware new-tab creation when the active shell reports a directory;
- `Ctrl+Shift+T` and `Ctrl+Shift+W` tab creation/closing;
- `Ctrl+PageDown` and `Ctrl+PageUp` tab navigation;
- first-run onboarding for local-shell behavior, context indicators, keyboard shortcuts, and Development limits;
- durable onboarding completion/current-step state;
- interruption/resume behavior for incomplete onboarding; and
- Help replay without clearing saved completion state.

### Data, Security, and Privacy Boundary

The only new GoreeCloud-owned persistent state is the onboarding file at:

`$XDG_CONFIG_HOME/goreecloud-terminal/state.ini`

It stores only onboarding completion and current step. It is not a command-history store, session-restore format, credential store, remote-host store, policy source, or authorization signal.

The integrated slice adds no SSH/remote transport, reusable credentials, telemetry, analytics, hidden network reporting, or automatic privilege escalation. Tab labels and context indicators remain informational.

### Remaining Product Boundary

This event does not establish:

- split panes;
- searchable scrollback or safe multiline-paste review;
- profiles or general settings;
- workspace/session persistence;
- robust shell command-boundary integration;
- SSH/remote administration;
- container workflows;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or release acceptance; or
- Stable/Anchor status.

Repository protection and the blank GitHub Description also remain separately unresolved and are tracked in GitHub issue #3.

## 2026-09-27 — Nested Split Panes Integrated

### Event

Pull request #6 integrated nested local split panes into the Development workspace model.

### Exact Source Evidence

- authoritative base: `e2c58c5ce9e55400e8f5d87c5f9dd6a4ee623a42`;
- initial pane candidate: `e03926a811e69d4fc5e28f05e2bfa0dceab1208d`;
- exact accepted candidate: `03ac28ba2a6db46ef11bdaa7483606e40b4c35ec`;
- pull request: #6, **Add nested split panes**;
- merge method: squash;
- authoritative merged commit: `003a9c3fd2b90f72f11646c9236950cbd85db081`;
- merged tree: `9ad0424f0f9d1dd3734f4e60e379b592780550b9`; and
- GitHub reports the merged commit signature as verified.

### Validation and Corrective Evidence

The first pane candidate compiled, but its new Xvfb-backed workspace test was terminated before assertions because GTK emitted an accessibility-bus warning that GLib's test harness treated as fatal:

- Native Foundation run `36352823241` — push — failed in test;
- Native Foundation run `36352849933` — pull request — failed in test.

The runtime test was preserved. The candidate was corrected by isolating the headless GUI test environment with `GTK_A11Y=none` and the Cairo renderer rather than weakening or deleting the test.

The corrected exact candidate then passed:

- Native Foundation run `36352934513` — push — success;
- Native Foundation run `36352938087` — pull request — success.

The merged authoritative revision passed:

- Native Foundation run `36353011758` — push on `main` — success.

The passing workflow verified repository governance, Meson configuration, native compilation, context tests, onboarding-state tests, the Xvfb workspace split/collapse test, and install layout.

### Implemented Development Boundary

Verified source now adds:

- nested `GtkPaned` layout trees inside tabs;
- left/right and top/bottom pane splitting;
- working-directory-aware pane creation;
- draggable split dividers;
- active-pane synchronization for tab label, window title, working-directory context, and session status;
- previous/next pane focus cycling;
- active-pane closing with surrounding split collapse; and
- onboarding/help and keyboard-shortcut updates for the pane model.

Pane-layout and focus state remain ephemeral and are intentionally not a workspace/session persistence format.

### Security and Privacy Boundary

Every pane remains a local shell session. This slice adds no SSH/remote transport, credentials, telemetry, analytics, network reporting, or persistent pane/session metadata. Pane labels, layout, focus, and context indicators remain informational and are not authorization signals.

### Remaining Product Boundary

This event does not establish:

- persistent workspace/session restoration;
- profiles or general settings;
- searchable/regex scrollback or command-boundary navigation;
- safe multiline-paste review;
- SSH/remote administration;
- container workflows;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or release acceptance; or
- Stable/Anchor status.

Repository protection and the blank GitHub Description remain separately unresolved and are tracked in GitHub issue #3.

