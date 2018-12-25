legislation.gov.uk parser
=========================

This program takes as an argument an url from a legislative act from the
legislation.gov.uk website and outputs the connections between sections
inside the act.

For more information visit: http://www.legislation.gov.uk/developer

Compilation on Ubuntu 18.04
---------------------------
This project uses meson for building: http://mesonbuild.com/.

To build and run C code, install: 
* libcurl4-openssl-dev
* libxml2-dev
* libpcre
* libgraphviz-dev
* libigraph0-dev
* libpq-dev
* meson

and write
> cd c_src
> mkdir build
> meson build
> cd build
> ninja

To run python code, install:
* python3-pip
* python3-psycopg2
* spacy (pip3 install -U spacy && python3 -m spacy download en)

The program was written, compiled and tested in Ubuntu 18.04. The code is
supposed to be portably written and the libraries are supposed to be portable
as well.

Usage
-----

Write
> ./leg -h

to find out about how the program works.
