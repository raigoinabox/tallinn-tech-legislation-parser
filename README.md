legislation.gov.uk parser
=========================

This program takes as an argument an url from a legislative act from the
legislation.gov.uk website. Notice, that when browsing to an act in the
browser, the url ends with "contents". The "contents" part must be removed
before giving the url to my application.

For more information visit: http://www.legislation.gov.uk/developer

Compilation on Ubuntu
---------------------
Get
* libxml2
* libcurl
* libpcre.
* libgraphviz-dev
* sqlite3

and write
> make

The program was written, compiled and tested in Ubuntu 16.04. The code is
supposed to be portably written and the libraries are suppose to be portable
as well.

Usage
-----
write
> leg <legislation.gov.uk act url>
or
> ./leg <legislation.gov.uk act url>

Then the program will output the connection between sections inside the act
in a graphviz dot format. The output is whole. One can directly feed it dot
without modifications. References to outside acts are ignored.

The program achieves this with a mix of parsing the xml and parsing the text.
Mostly text. 

### Makefile
For testing comfort, Makefile has a target run_test. It will construct a
graph.dot from a certain url. From the graph.dot it will construct a graph.pdf
using graphviz dot tool and open the graph.pdf file with the default
application for viewing pdfs.

The url is hard coded in the Makefile. 
