project('GtkWnckGdkCairoPlayground', 'cpp',
  version : '0.0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

gtkdep = dependency('gtk+-3.0')
wnckdep = dependency('libwnck-3.0')

exe = executable('experiment-executable', 'main.cpp',
  install : true,
  cpp_args : '-DWNCK_I_KNOW_THIS_IS_UNSTABLE',
  dependencies : [gtkdep, wnckdep])

test('basic', exe)
