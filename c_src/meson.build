add_global_arguments('-Wextra', '-Wpedantic', language: 'c')

curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
sqldep = dependency('sqlite3')

dep = [curldep, xmldep, pcredep, gvcdep, sqldep]
src = files('command_line.c', 'leg.c', 'misc.c', 'safe_string.c', 'util.c',
	'legislation.c', 'printing.c', 'sections.c', 'web.c', 'save_dbu_compl.c',
	'doing_business.c', 'maps.c', 'print_leg.c', 'text_parser.c',
	'database.c', 'results_dao.c', 'arg_parsing.c')
