project('tallinn-tech-legislation-parser', 'c')
add_global_arguments('-Wextra', '-Wpedantic', '-Wno-unused-function', language: 'c')

curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
sqldep = dependency('sqlite3')

dep = [curldep, xmldep, pcredep, gvcdep, sqldep]
src = ['command_line.c', 'leg.c', 'misc.c', 'safe_string.c', 'util.c',
	'comp_dbu.c', 'legislation.c', 'printing.c', 'sections.c', 'web.c',
	'doing_business.c', 'maps.c', 'print_leg.c', 'text_parser.c', 'vectors.c']
executable('leg', src, dependencies : dep)

sqlprog = find_program('sqlite3')
custom_target('database_target',
	output: 'cache.db',
	input: 'schema.sql', 
	command: [sqlprog, '@OUTPUT@', '-init', '@INPUT@'])
