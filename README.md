# OpenGL Example

A small example program using SDL2, OpenGL and GLEW.
It draws a 2D bridge-like shape on the screen, and allows to move, rotate and scale it.

I made this a while ago because I wanted to try out OpenGL.
As such, I apologize for the unclean code (and the French comments everywhere...)

## Prerequisites

* A C compiler that can do C99
* GLEW, SDL2, OpenGL 3.3
* Pkgconfig

## Building

Simply run `make`. It will create a build directory.

I only tested it on Linux, but it would probably work on Windows with MSYS2 and on Mac with Homebrew to handle dependencies.

## Running

Run the `ogl` executable in the build directory.

The default controls are:
* Left/Right arrow keys to move the shape
* Up/Down to scale
* B/N to rotate left/right
* W to toggle wireframe rendering
* Escape to exit.

The program creates a `config.cfg` file in the working directory. You can modify it to customize key bindings.

## License

MIT.

