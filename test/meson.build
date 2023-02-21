test_dependencies = dependencies + dependency('cmocka')
workdir = meson.project_source_root()

test_args = executable('test_args',
                       ['test_args.c', '../args.c'],
                       include_directories: '..',
                       dependencies: test_dependencies)
test('args', test_args, workdir: workdir)

test_json = executable('test_json',
                       ['test_json.c', '../json.c', '../common.c'],
                       include_directories: '..',
                       dependencies: test_dependencies)
test('json', test_json, workdir: workdir)
