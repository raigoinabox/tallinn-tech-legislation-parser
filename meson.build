project('legislation', 'c')
add_global_arguments('-Wextra', '-Wpedantic', language: 'c')

curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
sqldep = dependency('sqlite3')

print_dep = [curldep, xmldep, pcredep, gvcdep, sqldep]
print_src = [ 'command_line.c', 'leg-print.c', 'printing.c', 'safe_string.c',
	'sections.c', 'text_parser.c', 'util.c', 'web.c']
executable('leg-print', print_src, dependencies : print_dep)

dbu_dep = []
dbu_src = [ 'leg-dbu.c' ]
executable('leg-dbu', dbu_src, dependencies : dbu_dep)

sqlprog = find_program('sqlite3')
custom_target('database_target',
	output: 'cache.db',
	input: 'schema.sql', 
	command: [sqlprog, '@OUTPUT@', '-init', '@INPUT@'])
