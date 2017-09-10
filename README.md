legislation.gov.uk parser
=========================

This program takes as an argument an url from a legislative act from the
legislation.gov.uk website and outputs the connections between sections
inside the act.

For more information visit: http://www.legislation.gov.uk/developer

Compilation on Ubuntu
---------------------
This project uses meson for building: http://mesonbuild.com/.

Get
* libxml2
* libcurl
* libpcre.
* libgraphviz-dev
* sqlite3
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
> ./leg <legislation.gov.uk act url>

Then the program will write into a default file the connections between
sections inside the act in pdf format. References to outside acts are ignored.

Write
> ./leg -h

for more information about different options.

The program achieves this with a mix of parsing the xml and parsing the text.
Mostly text.

### Makefile
For testing comfort, Makefile has a target run_test. It will construct a
graph.pdf from a certain url and open the graph.pdf file with the default
application for viewing pdfs.

The url is hard coded in the Makefile. 
