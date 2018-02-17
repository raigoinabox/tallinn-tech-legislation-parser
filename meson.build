project('tallinn-tech-legislation-parser', 'c')

subdir('c_src')

sqlprog = find_program('sqlite3')
custom_target('database_target',
	output: 'data.db',
	input: 'schema.sql', 
	command: [sqlprog, '@OUTPUT@', '-init', '@INPUT@'])

executable('leg', src, dependencies : dep)
