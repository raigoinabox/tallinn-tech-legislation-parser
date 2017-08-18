CFLAGS += -Wall -Wextra -Wpedantic -g -std=c11 -MMD

CFLAGS += $(shell pkg-config --cflags libcurl)
LDLIBS += $(shell pkg-config --libs libcurl)
CFLAGS += $(shell pkg-config --cflags libxml-2.0)
LDLIBS += $(shell pkg-config --libs libxml-2.0)
CFLAGS += $(shell pkg-config --cflags libpcre)
LDLIBS += $(shell pkg-config --libs libpcre)
CFLAGS += $(shell pkg-config --cflags libgvc)
LDLIBS += $(shell pkg-config --libs libgvc)
CFLAGS += $(shell pkg-config --cflags sqlite3)
LDLIBS += $(shell pkg-config --libs sqlite3)

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
