# ImpactSearcher
This is a high performance library for searching bolides in the atmospheres of Jupiter and Saturn.
The source code of a Qt GUI is availble at https://github.com/SSIP/QtSearcher. We're also planning
to create a command line client to improve the performance and not have Qt dependencies.

# Installation
We do not yet provide installation packages as the program is not 100% finished yet. Download links
will be available on https://ssip.info.

Requirements / Dependencies
* libpng16 (http://www.libpng.org/)
* zlib (http://www.zlib.net/)

# How to compile
To compile the source code you need the CMake and a C++ compiler. We usually use the GNU C++ compiler.
The following steps describe the full compilation procedure on Linux:
1. `> git clone https://github.com/SSIP/ImpactSeracher.git`
2. `> cd ImpactSearcher`
3. `> cmake src`
4. `> make`
5. `> make install`

# Contributing
We welcome contributions to this project. Please be aware that you must MIT license your contributions.
If you want to contribute, create a pull request to this repository. Usually we will provide feedback within a
few days.