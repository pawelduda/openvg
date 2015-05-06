# Experimental fork with sample program that crashes after some time.

https://github.com/ajstarks/openvg/issues/22

There are two cases when the program crashes:
1. The screen freezes and the process goes into D state.
2. Random shapes appear on the screen, the process doesn't freeze but slows down by a lot.

## Build and run

<i>Note that you will need at least 64 Mbytes of GPU RAM:</i>. You will also need the DejaVu fonts, and the jpeg and freetype libraries.
The indent tool is also useful for code formatting.  Install them via:

	pi@raspberrypi ~ $ sudo apt-get install libjpeg8-dev indent libfreetype6-dev ttf-dejavu-core

Next, build the library, program and test it:

	git clone git://github.com/pawelduda/openvg
	cd openvg
	make
	cd client
	make digital-signage
	./digital-signage

To install the shapes library as a system-wide shared library
	
	pi@raspberrypi ~/openvg $ make library
	pi@raspberrypi ~/openvg $ sudo make install

The openvg shapes library can now be used in C code by including shapes.h and fontinfo.h and linking with libshapes.so:

	#include <shapes.h>
	#include <fontinfo.h>

	pi@raspberrypi ~ $ gcc -lshapes anysource.c
