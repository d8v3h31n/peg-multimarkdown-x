#!/bin/sh

make clean
make
rm *.o

/usr/bin/i586-mingw32msvc-cc -c -I/usr/i586-mingw32msvc/include/glib-2.0 -I/usr/i586-mingw32msvc/lib/glib-2.0/include -Wall -O3 -ansi markdown*.c
/usr/bin/i586-mingw32msvc-cc markdown*.o -Wl,-L/usr/i586-mingw32msvc/lib/glib-2.0,--dy,--warn-unresolved-symbols,-lglib-2.0 -o multimarkdown.exe
