Cachetest
=========
Author:  Alex Szlavik
Email:	<aszlavik@uwaterloo.ca>

INTRODUCTION:
Cachetest is a small POSIX compliant appliation allowing for the characterization and study of contemporary memory hierarchies.


COMPILING:
The cachtest suite utilizes the gnu autoconf process for generating makefiles and determining
system header file compatibility.

If you wish to use Cachetest with PAPI support, use --with-papi=<ABSOLUTE PATH TO PAPI INSTALL>
If PAPI is globally installed and visible via your PATH settings, you do not need this option.
Alternatively, Cachetest ships with a custom PMU event interface, which is used if no PAPI
installation is found.

./configure
make
(optionally) make install

RUNNING:
Cachetest can be run directly via the binary which was installed, or via the testing scripts
found in scripts/ .
