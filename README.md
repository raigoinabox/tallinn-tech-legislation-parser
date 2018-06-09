legislation.gov.uk parser
=========================

This program takes as an argument an url from a legislative act from the
legislation.gov.uk website and outputs the connections between sections
inside the act.

For more information visit: http://www.legislation.gov.uk/developer

Compilation on Ubuntu
---------------------
This project uses meson for building: http://mesonbuild.com/.

Install
* libxml2
* libcurl
* libpcre.
* libgraphviz-dev
* sqlite3
* libigraph0-dev
* meson

and write
> mkdir build
> meson build
> cd build
> ninja

The program was written, compiled and tested in Ubuntu 16.04. The code is
supposed to be portably written and the libraries are supposed to be portable
as well.

Usage
-----

Write
> ./leg -h

to find out about how the program works.
