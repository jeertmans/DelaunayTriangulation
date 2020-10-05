Doing animations in C with `BOV`
=======================================

This page teaches you how to use the BOV library (contained in
[deps/BOV/](deps/BOV/)) in order to make a beautiful visualization
for your upcoming project.

 * [FAQ](#faq)
 * [Tutorial](#tutorial)
	- [Creating a window](#creating-a-window)
	- [A window has its own time](#a-window-has-its-own-time)
	- [Hello World](#hello-world)
	- [Hello real world](#hello-real-world)

## FAQ

**Why C ?**

The first question that you might ask is:

> Why is the Advanced Numerical Method Project in C and not in
> Python, Java, C++, Rust or Julia ?

You will realize that C is very transparent about memory allocation.
Every non-constant chunk of memory that you use must be allocated on
the heap via a `malloc()` call. Therefore, you will realize precisely
how much memory is consumed by your algorithm, and you will naturally
avoid unnecessary allocations. That transparency and simplicity give
C a clear advantage over languages with fancy (but costly) features.
Finally, a well-realized C program will be as fast as any C++ or
Rust, and much faster than any interpreted language.

**What is BOV ?**

It is a small, simple OpenGL library created by your assistants to
help you draw lines, text, and other simple 2D shapes. All functions are
defined and briefly explained in
[deps/BOV/include/BOV.h](deps/BOV/include/BOV.h).
The most useful ones are described in the tutorial below. We figured
it would be easier than using something like CSFML or BGFX, and
lighter than requiring the SDL.



## Tutorial

First, open
[deps/BOV/include/BOV.h](deps/BOV/include/BOV.h).
This file contain the prototypes of every function that the BOV
library defines. If you want to draw a shape, give a color to it, a
position, a size, you will need to check those prototypes and the
handy little comments that accompany them. *BOV.h* some
functions that are used by BOV internally. **You should never use
the structures defined after the 1000th line directly**.

You can now open [src/main.c](src/main.c) and start coding.


### Creating a window

First, we will create a simple window (an object of type
`bov_window_t`) via a call to the function
```C
    bov_window_t* bov_window_new(int width,
                                 int height,
                                 const char* win_name);
```
where
 * **width** is the width of the window in pixels or 0 for
   full-screen
 * **height** is the height of the window in pixels or 0 for
   full-screen
 * **win_name** is a string which will be the name of your window

we will wait for 2 seconds using the function `time()` and
`difftime()` given in
[`<time.h>`](http://www.cplusplus.com/reference/ctime/)

and close the window (and deleted all structure related to the
window) with
```C
    void bov_window_delete(bov_window_t* window);
```


Therefore, in the following code, we create a window of size 800x800
titled "Tutorial 0", we wait 2 seconds, close the window and return.
```C
#include "BOV.h"
#include <time.h>

int main()
{
    bov_window_t* window = bov_window_new(800, 800, "Tutorial 0");

    time_t tic = time(NULL);

    while(difftime(time(NULL) , tic)< 2)
        ;// do nothing

    bov_window_delete(window);

    return 0;
}
```


### A window has its own time

Before drawing things into a window, we have to understand that the
window object is actually much more than a window. Indeed, when you
will draw primitive shapes with `bov_points_draw()`,
`bov_text_draw()`... you will in fact draw to a texture in the memory
of your computer which is called a framebuffer. More specifically,
the framebuffer that is shown on your screen is the front buffer and
the framebuffer that you are drawing to is called the back buffer.
Once you've filled the framebuffer with what you want to draw, you
can show it in the window using:
```C
void bov_window_update(bov_window_t* window)
```
That function actually does a lot of things:

 - it swaps the back buffer (the texture with your drawings) with the
   front buffer.
 - it waits for a screen refresh if you have VSYNC. A refresh is the
   moment when the screen buffer is actually shown on your screen. It
   depends on the refreshing rate (framerate) of your screen (usually
   60 Hz)
 - it processes events (mouse inputs and keyboard inputs) that
   happened since the last call to `bov_window_update()`.
 - it updates the size of the framebuffer in pixel according to the
   size of the window.
 - **it clears the back buffer**
 - it changes the render position, change the scaling (zoom), pause
   the window timer, tell if the window should close etc. according
   to the event received.
 - it updates its own timer if the window is not paused


Now, let's update our previous example using the window timer:
```C
#include "BOV.h"

int main()
{
    bov_window_t* window = bov_window_new(800, 800, "mY wInDoW");

    while(bov_window_get_time(window) < 2) {
        bov_window_update(window);
    }

    bov_window_delete(window);

    return EXIT_SUCCESS;
}
```

Because we use the timer of the window, **we can pause the time by
pressing the space bar**. If you do so before the 2 seconds delay,
the window won't close because the time will get stuck somewhere in
between 0 and 2 seconds. Pressing the space bar again will restart
the timer and the window will close.

You might also notice that the background color is now white, which
is the default color for the background of the framebuffer. In the
previous example, it stayed black because we were never showing the
content of the framebuffer to the window.

You can change the background color of the window using:
```C
void bov_window_set_color(bov_window_t* window, float rgba[4]);
```

The color `rgba` is defined by 4 floating-point values `float` in the
range 0.0 - 1.0.
 1. red
 2. green
 3. blue
 4. alpha (the transparency)

Note that the transparency is totally useless with the window
background color, because transparency only has an impact when things
are drawn behind our object.

---

**waiting for user to close the window**

Showing a window during 2 second is not very useful. You will
generally want to display the window until the user clicks the close
button, press the escape key or ALT+F4. Those events are captured by
the window when using `bov_window_update()`. You can simply ask the
window if they happened by calling
```C
int bov_window_should_close(bov_window_t* window)
```
in this manner

```C
while(!bov_window_should_close(window)) {
    // draw stuff here

    // ...

    bov_window_update(window);
}
```



### Hello World

Time to render our first primitive!

Let's use the function `text_new()`, which returns a new text
object:
```C
bov_text_t* bov_text_new(GLubyte* string, GLenum usage);
```
 - **string** is the message that you want to display
 - **usage** is either GL_DYNAMIC_DRAW if you intend to change the
   content (the text) of the object regularly or GL_STATIC_DRAW if
   the text won't change.

Additionally, there are multiple parameters associated to a text
object that you can change: its position on the screen, its color,
its width, its outline color, its outline width, its scaling in x and
y... see [deps/BOV/include/BOV.h](deps/BOV/include/BOV.h) for more
info.


To render our text object into the framebuffer, we have to call
```C
void bov_text_draw(bov_window_t* window, bov_text_t* text);
```
Reminder: we will have to do this repeatedly because
`bov_window_update()` clears the framebuffer.


Finally, when we don't need our text object anymore, we will call
```C
void bov_text_delete(bov_text_t* text);
```
to destroy it.



Using all your knowledge, you should be able to display "Hello world"
with some pretty effects, by varying the text object parameters.
```C
#include "BOV.h"
#include <math.h>

#define PI 3.14159265358979323846

void nice_colormap(float color[4], float a)
{
    color[0] = sin(0.33 * a) * 0.3 + 0.7;
    color[1] = sin(0.23 * a + 2.) * 0.3 + 0.7;
    color[2] = sin(0.17 * a + 5.) * 0.3 + 0.6;
    color[3] = 1.0; // solid
}

int main()
{
    bov_window_t* window = bov_window_new(800, 800, "Tutorial 1");
    bov_window_set_color(window, (GLfloat[]) {0.5, 0.5, 0.5, 1.0});

    // hw is prefix for hello world :p
    bov_text_t* hw_obj = bov_text_new((GLubyte[]) {"Hello World !"},
                                      GL_STATIC_DRAW);

    float hw_color[4] = {1.0, 1.0, 1.0, 1.0};
    bov_text_set_fontsize(hw_obj, 0.2); // character is 0.2 tall (0.1 is the default)

    // a character is twice taller than it is large => width = 0.1
    // centering "Hello World !": 13/2=6.5 characters => 0.65
    bov_text_set_pos(hw_obj, (GLfloat[2]) {-0.65, 0.0});
    bov_text_set_boldness(hw_obj, 0.4); // bold
    bov_text_set_outline_width(hw_obj, 1.0); // big outline
    bov_text_set_outline_color(hw_obj, (float[4]) {0, 0, 0, 1});


    while(!bov_window_should_close(window)) {
        nice_colormap(hw_color, window_get_time(window));
        bov_text_set_color(hw_obj, hw_color);
        bov_text_draw(window, hw_obj);

        bov_window_update(window);
    }

    bov_text_delete(hw_obj);
    bov_window_delete(window);

    return EXIT_SUCCESS;
}
```

This basic program is part of the examples included with the BOV
library. Its source code,
[deps/BOV/examples/hello_world.c](deps/BOV/examples/hello_world.c),
is compiled automatically when the library is compiled, which also
happens automatically when you compile your program.
The executable should be located in
```sh
./build/deps/BOV/examples/hello_world
```

Among the other examples included with BOV, two are of particular
interest for drawing text:

 - [deps/BOV/examples/character_table](deps/BOV/examples/character_table.c)
   shows you all the available characters, as well as how to use the
   set of non-ASCII characters available.
 - [deps/BOV/examples/text_style](deps/BOV/examples/text_style.c)
   shows you how you can affect the text appearance.


### Adding files and changing the executable name

The [CMakeList.txt](CMakeList.txt) file is the blueprint to compile the
code into an executable file. You can change the name of the executable
by changing `myexec` to whatever name you want. You can also add any `.c`
file to your project by adding them below

    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c".

The C headers (`.h` files) are automatically added to the project by CMake
which automatically reads the `#include` directives and builds a
dependency graph.
