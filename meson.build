project('tallinn-tech-legislation-parser', 'c')
add_global_arguments('-Wextra', '-Wpedantic', language: 'c')

subdir('c_src')
subdir('r_project')

leg_exe = executable('leg', c_src, dependencies : c_dep)

sqlprog = find_program('sqlite3')
custom_target('database_target',
	output: 'data.db',
	input: 'schema.sql', 
	command: [sqlprog, '@OUTPUT@', '-init', '@INPUT@'])

valgrind = find_program('valgrind')
benchmark('bench-dbu', valgrind,
	args: ['--tool=callgrind',  './leg', 'save-dbu-compl'])
