CFLAGS += -Wall -Wextra -Wpedantic -g -std=c11

CFLAGS += -I/usr/include/libxml2
LDLIBS += -lcurl -lxml2 -lpcre

CFLAGS += `pkg-config --cflags libgvc`
LDLIBS += `pkg-config --libs libgvc`
CFLAGS += `pkg-config --cflags sqlite3`
LDLIBS += `pkg-config --libs sqlite3`

object_files := leg.o text_parser.o sections.o printing.o web.o util.o command_line.o

all: leg tags cache.db

.PHONY : all clean run_test format
.DELETE_ON_ERROR:

clean:
	$(RM) leg $(object_files)
run_test : graph.pdf
	xdg-open $<

leg : $(object_files)
leg.o : leg.c text_parser.h printing.h sections.h web.h command_line.h
sections.o : sections.c text_parser.h
web.o: web.c util.h

graph.pdf: leg
	./$< -d www.legislation.gov.uk/ukpga/1998/29 > $@

tags : leg.c text_parser.c sections.c
	ctags -R

format:
	astyle --indent=tab --style=allman --max-code-length=80 --suffix=none --recursive '*.c' '*.h'

cache.db: schema.sql
	sqlite3 $@ < $<
