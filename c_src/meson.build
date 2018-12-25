project('tallinn-tech-legislation-parser', 'c',
	default_options: [
		'buildtype=debugoptimized',
		'warnlevel=3',
		'werror=true',
		'b_lto=true'])
add_global_arguments('-Wextra', '-Wpedantic', language: 'c')

curldep = dependency('libcurl')
xmldep = dependency('libxml-2.0')
pcredep = dependency('libpcre')
gvcdep = dependency('libgvc')
igraphdep = dependency('igraph')
postgresql = dependency('libpq')
mrss = dependency('mrss')

c_dep = [curldep, xmldep, pcredep, gvcdep, igraphdep, postgresql, mrss]
c_src = files('main.c',
	'arg_parsing.c',
	'convert_csv.c',
	'print_leg.c',
	'save_dbu_compl.c',
	'save_graph.c',
	'command_line.c',
	'legal_act.c',
	'strings.c',
	'util.c',
	'printing.c',
	'dao.c',
	'database.c',
	'doing_business.c',
	'error.c',
	'estonia.c',
	'files.c',
	'maps.c',
	'leg_feed.c',
	'sections.c',
	'text_parser.c',
	'vectors.c',
	'web.c')

leg_exe = executable('leg', c_src, dependencies : c_dep)
