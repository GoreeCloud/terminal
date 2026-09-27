context_test = executable(
  'test-context',
  [
    'test-context.c',
    '../src/gc-context.c',
  ],
  include_directories: include_directories('../src'),
  dependencies: [glib_dep],
)

test('context helpers', context_test)
