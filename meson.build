project('legislation', 'c')

curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
sqldep = dependency('sqlite3')
dependencies = [curldep, xmldep, pcredep, gvcdep, sqldep]

src = [ 'command_line.c', 'leg.c', 'printing.c', 'safe_string.c',
	'sections.c', 'text_parser.c', 'util.c', 'web.c']

add_global_arguments('-Wextra', '-Wpedantic', language: 'c')

executable('leg', src, dependencies : dependencies)

sqlprog = find_program('sqlite3')
custom_target('database_target',
	output: 'cache.db',
	input: 'schema.sql', 
	command: [sqlprog, '@OUTPUT@', '-init', '@INPUT@'])
