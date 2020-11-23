# Numerical Geometry Project

- Author: Jérome Eertmans (13551600)
- Course: Numerical Geometries (LMECA2710)
- Algorithm: Delaunay triangulation


## How to use this project

If you want to use the project, you need to first compile it:

```
cd build
make
cd ..
```

Then you can execute the program. A variety of parameters is available, please use `-h` flag to see them.
```
./build/bin/lmeca2710_project
```


**However**, I **highly** recommend you to use my Python wrapper, as it adds
very nice sound effects!

What you need:
    - A Python3 installation
    - 2 small Python packages (use: `pip3 install -r requirements.txt`)

Then, you can use the produce the exact same results + some bonuses:
```
python3 main.py [-param value] ...
```

You can also modify the music theme in `main.py`.

## Sound bibliography

All the musics are Copyright Free songs.
If you wan to reuse them, please make sure to keep it as a private usage.

Sound effets:
http://soundbible.com

Synthwave theme:
https://www.youtube.com/watch?v=onEyXlpdY3U

Chill theme:
https://incompetech.com/music/royalty-free/index.html?isrc=USUAN1100326

## Old README

This folder is the canvas for your upcoming project.
It should contain:
 * this file (**README.md**)
 * the description of the structure of the program in **CMakeLists.txt**
 * a **src** directory containing the the source code of your program
 * a **doc** directory containing more documentation
 * a **deps** directory containing the BOV library

See [doc/COMPILING.md](doc/COMPILING.md) for a step by step tutorial
on how to build the program.

See [doc/tutorial.md](doc/tutorial.md) for a step by step tutorial on
how to use the BOV library.

See [deps/BOV/include/BOV.h](deps/BOV/include/BOV.h)
for help on the BOV library functions.

See [deps/BOV/examples/](deps/BOV/examples/) for more
examples using the BOV library
