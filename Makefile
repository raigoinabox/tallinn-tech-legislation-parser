CFLAGS += -Wall -Wextra -Wpedantic -g -std=c11 -MMD

CFLAGS += `pkg-config --cflags libcurl`
LDLIBS += `pkg-config --libs libcurl`
CFLAGS += `pkg-config --cflags libxml-2.0`
LDLIBS += `pkg-config --libs libxml-2.0`
CFLAGS += `pkg-config --cflags libpcre`
LDLIBS += `pkg-config --libs libpcre`
CFLAGS += `pkg-config --cflags libgvc`
LDLIBS += `pkg-config --libs libgvc`
CFLAGS += `pkg-config --cflags sqlite3`
LDLIBS += `pkg-config --libs sqlite3`

object_files := $(patsubst %.c,%.o,$(wildcard *.c))

all: leg tags cache.db

.PHONY : all clean run_test format
.DELETE_ON_ERROR:

clean:
	$(RM) leg *.o *.d
run_test : graph.pdf
	xdg-open $<
format:
	astyle --indent=tab --style=allman --max-code-length=80 --suffix=none --recursive '*.c' '*.h'

-include $(object_files:.o=.d)

leg : $(object_files)

graph.pdf: leg
	./$< -g www.legislation.gov.uk/ukpga/1998/29 > $@
tags : *.c
	ctags -R
cache.db: schema.sql
	sqlite3 $@ < $<
