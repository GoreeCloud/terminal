context_test = executable(
  'test-context',
  [
    'test-context.c',
    '../src/gc-context.c',
    '../src/gc-shell-state.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep],
)

onboarding_state_test = executable(
  'test-onboarding-state',
  [
    'test-onboarding-state.c',
    '../src/gc-onboarding-state.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep],
)

link_utils_test = executable(
  'test-link-utils',
  [
    'test-link-utils.c',
    '../src/gc-link-utils.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep, gio_dep],
)

paste_safety_test = executable(
  'test-paste-safety',
  [
    'test-paste-safety.c',
    '../src/gc-paste-safety.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep],
)

shell_state_test = executable(
  'test-shell-state',
  [
    'test-shell-state.c',
    '../src/gc-shell-state.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep],
)

workspace_test = executable(
  'test-workspace',
  [
    'test-workspace.c',
    '../src/gc-workspace.c',
    '../src/gc-terminal-session.c',
    '../src/gc-link-utils.c',
    '../src/gc-context.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [gtk_dep, vte_dep, glib_dep, gio_dep, pcre2_dep],
)

workspace_test_env = environment()
workspace_test_env.set('GTK_A11Y', 'none')
workspace_test_env.set('GSK_RENDERER', 'cairo')

test('context helpers', context_test)
test('onboarding state', onboarding_state_test)
test('link target validation', link_utils_test)
test('paste safety', paste_safety_test)
test('shell state', shell_state_test)
test('workspace panes', workspace_test, env: workspace_test_env)
