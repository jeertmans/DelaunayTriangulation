# Numerical Geometry Project

- Author: Jérome Eertmans (13551600)
- Course: Numerical Geometries (LMECA2710)
- Algorithm: Delaunay triangulation

<p align="center"> <img src="static/lmeca2170.png" width="300"></p>

## How to use this project

If you want to use the project, you need to first compile it:

```
cd build
make
cd ..
```

Alternatively, you can use `./buid.sh` if you are on a UNIX machine.

Then you can execute the program. A variety of parameters is available, please use `-h` flag to see them.
```
./build/bin/lmeca2710_project [-param value] ...
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

**Warning**: key bindings assume you have an AZERTY layout. If your keyboard layout differs, please transpose the keys accordingly.
Example: pressing *Q* on a QWERTY layout will be understood as the key *A*.

## Drawing images with triangulation

Some funny images can be generated by triangulating very specific sets of points.
Please have a look into the *input* folder.

The drawings data files were kindly generated by Pierre Veldeman using some public image found on the internet.

I edited some files manually to show you what you can do with this program.



<p align="center"> <img src="static/love.png" width="300"></p>

## Known issues

In the GUI, using the SHIFT + KEY (i.e. fast repeat of KEY) can cause memory problems.

This is the case when Voronoi and pretty drawings are activated, and that you try to
quickly delete a lot of points.

## Sound bibliography

All the musics are Copyright Free songs.
If you wan to reuse them, please make sure to keep it as a private usage.

Sound effects:
http://soundbible.com

Synthwave theme:
https://www.youtube.com/watch?v=onEyXlpdY3U

Chill theme:
https://incompetech.com/music/royalty-free/index.html?isrc=USUAN1100326

## Guides

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
