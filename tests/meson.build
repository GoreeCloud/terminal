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

test('context helpers', context_test)
test('onboarding state', onboarding_state_test)
