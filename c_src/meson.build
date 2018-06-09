curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
sqldep = dependency('sqlite3')
igraphdep = dependency('igraph')

c_dep = [curldep, xmldep, pcredep, gvcdep, sqldep, igraphdep]
c_src = files('command_line.c', 'leg.c', 'misc.c', 'strings.c', 'util.c',
	'legislation.c', 'printing.c', 'sections.c', 'web.c', 'save_dbu_compl.c',
	'doing_business.c', 'maps.c', 'print_leg.c', 'text_parser.c',
	'database.c', 'results_dao.c', 'arg_parsing.c', 'convert_csv.c',
	'vectors.c')
