BOV
===

BOV stands for "Basic OpenGL Viewer", it is a very thin wrapper over OpenGL
with the goal to make it very easy to draw anti-aliased points/lines/triangles
with border and multiple other effects.


## Compiling the library

        mkdir build
        cd build
        cmake -DBOV_BUILD_EXAMPLES=ON ..
        make


## Examples

There are multiples examples within the *examples* directory.
The examples are automatically compiled together with the library, except if
you disable that with the cmake option `-DBOV_BUILD_EXAMPLES=OFF`

For some examples, running the code is actually more explicative than the code
itself, so I recommend running an example before looking at its code.


## License

This software is under the zlib/libpng license. You should have received a copy
of this license in the same folder as this file.

https://opensource.org/licenses/Zlib


## Author

Célestin Marot (marotcelestin@gmail.com)

