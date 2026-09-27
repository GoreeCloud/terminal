context_test = executable(
  'test-context',
  [
    'test-context.c',
    '../src/gc-context.c',
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

workspace_test = executable(
  'test-workspace',
  [
    'test-workspace.c',
    '../src/gc-workspace.c',
    '../src/gc-terminal-session.c',
    '../src/gc-context.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [gtk_dep, vte_dep, glib_dep, gio_dep],
)

workspace_test_env = environment()
workspace_test_env.set('GTK_A11Y', 'none')
workspace_test_env.set('GSK_RENDERER', 'cairo')

test('context helpers', context_test)
test('onboarding state', onboarding_state_test)
test('workspace panes', workspace_test, env: workspace_test_env)
