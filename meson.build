project('iso-chooser-menu', 'c',
        meson_version: '>=0.60.0',
        default_options: ['werror=true'])

arch= run_command('dpkg-architecture', '-q', 'DEB_BUILD_ARCH',
                  check:true).stdout().strip()
add_global_arguments(['-DARCH="@0@"'.format(arch), '-Wfatal-errors'],
                     language:'c')

srcs = ['main.c', 'args.c', 'common.c', 'json.c']
dependencies = [dependency('ncursesw'), dependency('json-c')]

menu = executable('iso-chooser-menu',
                  srcs,
                  dependencies:dependencies,
                  install:true,
                  install_dir:'/usr/lib/mini-iso-tools')

subdir('test')
