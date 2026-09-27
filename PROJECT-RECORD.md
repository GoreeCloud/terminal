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

## 2026-09-27 — Scrollback Search and Guarded Clipboard Paste Integrated

### Event

Pull request #8 integrated the next bounded Development slice: active-pane scrollback search plus explicit review before multiline clipboard paste is delivered to a local shell.

### Exact Source Evidence

- authoritative base: `528ed4f4f7e6e6dbbb7a1967df9d0f7864db886d`;
- accepted exact candidate: `738bc4f80c9086aa4bd5965fac98107d01fe0a6f`;
- pull request: #8, **Add scrollback search and guarded clipboard paste**;
- merge method: squash;
- authoritative merged commit: `4f2114917eec6e6850eb6ddfd95ef3ecb3581201`;
- merged tree: `795ea5d686a55764f1f852dee882e3062c9bd154`; and
- GitHub reports the merged commit signature as verified.

### Validation Evidence

The accepted exact candidate passed:

- Native Foundation run `36354142044` — push — success;
- Native Foundation run `36354144379` — pull request — success.

The merged authoritative revision then passed:

- Native Foundation run `36354208767` — push on `main` — success.

The workflow verified repository governance, Ubuntu 24.04 dependency setup, Meson configuration, native compilation, context tests, onboarding-state tests, paste-safety tests, the Xvfb-backed workspace test, and install layout.

Earlier candidate revisions were superseded and are not acceptance evidence for the merged head.

### Implemented Development Boundary

Verified source now adds:

- active-pane scrollback search opened with `Ctrl+Shift+F`;
- literal search by default with optional regular-expression and case-sensitive modes;
- wrap-around next/previous match navigation with `Ctrl+G` and `Ctrl+Shift+G`;
- inline invalid-expression and no-match feedback;
- guarded GTK clipboard reads for an explicit paste request;
- interception of VTE's clipboard-paste route so Shift+Insert uses the same review path;
- mandatory modal review whenever clipboard text contains a carriage return or line feed;
- exact paste text plus line and character counts in the review;
- explicit Cancel / Paste anyway choice; and
- VTE paste-text delivery after confirmation so VTE paste processing, including bracketed-paste behavior where applicable, remains available.

### Security and Privacy Boundary

Clipboard text is read only after an explicit clipboard-paste request. GoreeCloud Terminal does not persist the reviewed clipboard content. Search patterns are process-local and are not persisted or transmitted.

The review is a deliberate-execution guard, not a determination that reviewed commands are safe.

Primary-selection middle-click paste is **not** intercepted by this slice and remains explicitly outside the claimed protection boundary. Search and paste handling add no remote transport, reusable credentials, telemetry, analytics, or network reporting.

### Remaining Product Boundary

This event does not establish:

- primary-selection multiline-paste review;
- command-boundary navigation;
- URL/path interaction;
- profiles or general settings;
- persistent workspace/session restoration;
- robust shell command-start/completion/exit-status/process integration;
- SSH/remote administration;
- container workflows;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or release acceptance; or
- Stable/Anchor status.

Repository protection and the blank GitHub Description remain separately unresolved and are tracked in GitHub issue #3.

## 2026-09-27 — Validated URL and Local-Path Interaction Integrated

### Event

Pull request #10 integrated deliberate Ctrl+click interaction for terminal hyperlinks, allowlisted URL targets, and existing local paths.

### Exact Source Evidence

- authoritative base: `2a793a7dda079d2b64a3a813beef799f525fe3e5`;
- first source candidate: `199468735a303034108374033a4ffadee2c421ac`;
- header-corrected candidate: `92073f997c3a74cc057061c12710957952416573`;
- PCRE2-corrected candidate: `04366282742ce4f86253dc027c8d7e67a75f714b`;
- exact accepted/hardened candidate: `d33a20a7fd64e873a0fc0d538bbefe574635b97c`;
- pull request: #10, **Add validated URL and path interaction**;
- merge method: squash;
- authoritative merged commit: `df7111cac1fcb0f911809ce610b7b156a8fb2c37`;
- merged tree: `39c34da58b61f6cab886d45fcea6ece7fb997421`; and
- GitHub reports the merged commit signature as verified.

### Corrective and Validation Evidence

The first source candidate failed compilation because a generated header edit contained literal `\n` text instead of real line breaks:

- Native Foundation run `36354961912` — push — failed in compile.

That generated-text defect was corrected without weakening validation. The next candidate compiled, but the Xvfb workspace test exposed VTE's runtime requirement that terminal match regexes be compiled with PCRE2 multiline flags:

- Native Foundation run `36355084004` — push — failed in test;
- Native Foundation run `36355088167` — pull request — failed/cancelled as the candidate was superseded.

The runtime test was preserved. PCRE2 was declared explicitly and match regexes were compiled with the required multiline flag. The corrected candidate then passed:

- Native Foundation run `36355220056` — push — success;
- Native Foundation run `36355224222` — pull request — success.

The candidate was subsequently hardened to reject target control characters and reduce URL/path-regex overlap. The exact accepted head passed:

- Native Foundation run `36355274565` — push — success;
- Native Foundation run `36355277161` — pull request — success.

After merge, authoritative `main` commit `df7111cac1fcb0f911809ce610b7b156a8fb2c37` passed:

- Native Foundation run `36355339618` — push on `main` — success.

The passing workflow verified repository governance, Meson configuration, native compilation, context tests, onboarding-state tests, link-target validation tests, paste-safety tests, the Xvfb workspace integration test, and install layout.

### Implemented Development Boundary

Verified source now adds:

- Ctrl+primary-click intent before link/path opening;
- explicit OSC 8 hyperlink lookup through VTE;
- bounded HTTP/HTTPS/mailto/file and local-path match expressions;
- an HTTP/HTTPS/mailto/local-file URI allowlist;
- rejection of unsupported schemes and target control characters;
- rejection of remote `file://` hosts;
- resolution of explicit absolute, `~/`, `./`, and `../` local paths using active-pane working-directory context;
- local-path existence validation before desktop launch;
- GIO desktop-default-handler launch for validated targets; and
- focused GLib/GIO tests for scheme, URI, control-character, path, and missing-path validation.

PCRE2 is now an explicit compile-time dependency for the multiline flag required by VTE terminal match regexes.

### Security and Privacy Boundary

Terminal output and OSC 8 targets are treated as untrusted input. Matched text is never executed through the shell by this path. External opening requires an explicit Ctrl+click, arbitrary URI schemes are not launched, local paths must exist, remote file hosts are rejected, and GoreeCloud Terminal does not persist link/path history.

Opening an allowed HTTP/HTTPS/mailto target hands it to the configured desktop application; any later network or external-application behavior is outside the Terminal transport boundary and occurs only after the user's explicit open request.

### Remaining Product Boundary

This event does not establish:

- trustworthy command-boundary navigation or broader shell integration;
- guarded primary-selection middle-click paste;
- bare-filename guessing;
- remote file URI opening;
- SSH/remote administration;
- profiles or general settings;
- persistent workspace/session restoration;
- container workflows;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or release acceptance; or
- Stable/Anchor status.

Command-boundary navigation remains gated on trustworthy shell integration rather than prompt-text inference. Repository protection and the blank GitHub Description remain separately unresolved in issue #3.

## 2026-09-27 — Guarded Primary-Selection Paste Integrated

### Event

Pull request #12 closed the remaining guarded-paste source gap by routing primary-selection middle-click paste through the same GoreeCloud review path used for clipboard paste.

### Exact Source Evidence

- authoritative base: `c3ecdd631ee1b42cbfd591c8ee325e0c42f67528`;
- source/test candidate: `105990381ef6ef6bcce3d2892ac17b674a9eb74a`;
- exact accepted candidate: `acac82544cb346a0247467e61028f7fc4ef982cb`;
- pull request: #12, **Guard primary-selection middle-click paste**;
- merge method: squash;
- authoritative merged commit: `9ee3a2bd35b7ffbdf2cf1734ea06f8a3619f6b2b`;
- merged tree: `579777d563afcee46705a70e4a3643c6394e7619`; and
- GitHub reports the merged commit signature as verified.

### Validation Evidence

The exact accepted candidate passed:

- Native Foundation run `36356183353` — push — success;
- Native Foundation run `36356199632` — pull request — success.

After merge, authoritative `main` commit `9ee3a2bd35b7ffbdf2cf1734ea06f8a3619f6b2b` passed:

- Native Foundation run `36356436836` — push on `main` — success.

The workflow verified repository governance, Ubuntu 24.04 dependency setup, Meson configuration, native compilation, context tests, onboarding-state tests, link-target tests, paste-safety tests, the Xvfb workspace integration test including paste-source routing, and install layout.

### Implemented Development Boundary

Verified source now adds:

- an explicit clipboard-versus-primary paste-source model;
- capture-phase middle-button routing for primary-selection paste;
- GTK primary-clipboard reads for that source;
- reuse of the existing active-session validation, asynchronous text read, multiline-review modal, and VTE paste-text delivery path;
- preservation of ordinary terminal text selection because only middle-button paste is claimed; and
- Xvfb-backed routing coverage for both clipboard and primary-selection paste sources.

### Security and Privacy Boundary

Single-line primary-selection text remains direct paste through the same VTE paste-text delivery path used for accepted clipboard text. Text containing a carriage return or line feed requires explicit review before delivery to the shell.

The review remains a deliberate-execution guardrail, not a determination that confirmed commands are safe. Primary-selection contents are not persisted by GoreeCloud Terminal. This slice adds no telemetry, remote transport, credential handling, command analysis, or command-boundary inference.

### Remaining Product Boundary

This event does not establish:

- trustworthy shell command-boundary navigation;
- a general shell-integration protocol;
- SSH/remote administration;
- profiles or general settings;
- persistent workspace/session restoration;
- container workflows;
- representative Linux desktop runtime acceptance;
- complete Glaze UI 1.6.0 consumer conformance;
- production packaging or release acceptance; or
- Stable/Anchor status.

Trustworthy command-boundary navigation remains gated on shell integration that does not infer privileged state from spoofable prompt or terminal-output text. Repository protection and the blank GitHub Description remain separately unresolved in issue #3.

