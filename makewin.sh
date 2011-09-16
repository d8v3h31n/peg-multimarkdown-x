#!/bin/sh

make clean
make
rm *.o

/usr/bin/i586-mingw32msvc-cc -c -Wall -O3 -ansi markdown*.c GLibFacade.c
/usr/bin/i586-mingw32msvc-cc markdown*.o GLibFacade.o -Wl,--dy,--warn-unresolved-symbols,-lglib-2.0 -o multimarkdown.exe
