V4P - Vectors For Pocket - is a minimalistic layered polygons scan-conversion engine.

=Introduction=

V4P is a ridiculously short piece of C code. V4P main engine weights ~1000 instructions.

V4P targets very light platforms without advanced hardware resources.

V4P may be easily embedeed in any GUI or Game development library or application.

V4P has no dependencies with any library. V4P brings its own integer only maths routines.

V4P only asks you to provide an horizontal line drawing function (a memset()-like function should work well in most cases).

Beside rendering vectors scene, V4P will help you find collides.

=How to use it=

V4P is a set of C files and their related header files.

They can be compiled and linked into some library or directly added to your own project.

Warning: provided makefiles deal with Palm OS 4 gcc based compilation.

Two Palmphi projects are provided as well. Palmphi is an IDE running on Windows with cygwin which simplifies the authoring of Palm-OS based applications. See: http://www.digital-kingdoms.net/palmphi/

* palmphi-toto : A V4P based top-view car game. A work in progress.

* palmphi-totoe : A V4P based picture editor. Draw layered polygons based picture!

To compile these 2 projects, put the content of "src" directory into the path /usr/src/v4p/ of your cygwin distribution. Compile V4P sources before the application ones.

=Design Introduction=

The underlying algorithm of V4P can be named:
 "Bresenham-like iterative scanline and active edge cross-over computation based polygon scan conversion algorithm".

Additionaly, most V4P computed lists are quick-sorted. And bit-based computation helps V4P to find collides and pixel highest owner.

V4P design spares memory a lot. It doesn't deal with Z-buffer, not even "S-buffer" (as depicted here: http://www.gamedev.net/reference/articles/article668.asp).

V4P has been developped and tested on a very old Sony Clie PDA with Palm OS 4 gcc based SDK. V4P can be adapted after few tweakings on any embedeed linux devices and other geeky gadgets.


=Further experiments=

==writing more span drawing functions==

# More complex color type. blitting paint implementation.

# direct to linux frame buffer implementation

# SDL based implementation

# DirectFB based implementation

# V4P port for DS linux

# smartest drawing routine considering the Color type as a structure pointer giving a lot of data about the way to fill up a polygon.

# simple anti-aliasing solution through rescaling trick


==writing libraries on top of V4P==

# use V4P to design a Flash-like technology

# that is, write V4P bindings to known languages

# use V4P as a basis for a full animated GUI stack

==various improvements==

===transparent polygons===
this feature needs a change in the pixel parent election code so to call the span drawing routine from the highest opaque polygon to the highest translucent polygon up there.

===adding other edge shapes===

I miss circular curves a lot. But I didn't find a way to draw them in a sinle pass algorithm.

===increasing z depth range===

only 16 layers for now. Using a balanced binary tree of span parents could make z infinitivly more precise.

===Use more accurate cos/sin/atan/dist routines===

===avoid refreshing the static parts of the scene===
  * an integrated solution need an in-depth rewrite
  * an other solution is to call V4P repeatidly on "dirty rectangles"

=Otherelse=

Contact: sylvain.garden@gmail.com
