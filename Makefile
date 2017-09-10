build_dir := build

all: run_build tags

.PHONY : all run_test format run_build
.DELETE_ON_ERROR:

run_build:
	cd $(build_dir) && ninja
run_test: $(build_dir)/graph.pdf
	xdg-open $<
format:
	astyle --indent=tab --style=allman --max-code-length=80 --suffix=none '*.c' '*.h'

$(build_dir)/leg: run_build
$(build_dir)/graph.pdf: $(build_dir)/leg
	$< -g www.legislation.gov.uk/ukpga/1998/29 > $@
tags : *.c
	ctags -R
