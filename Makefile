ALL : multimarkdown

<<<<<<< .merge_file_mRSaeN
VERSION=3.0a43
=======
VERSION=3.0a4
>>>>>>> .merge_file_I7wrxl

PROGRAM=multimarkdown

ifeq ($(shell uname),Darwin)
	# For compiling on Mac OS X, at least with glib2 from fink which is
	# compiled for i386.  If it complains can remove the "-arch i386" part
	CFLAGS ?= -Wall -O3 -ansi -arch i386
else
	# Otherwise
	CFLAGS ?= -Wall -O3 -ansi
endif

OBJS=markdown_parser.o markdown_output.o markdown_lib.o
PEGDIR=peg-0.1.4
LEG=$(PEGDIR)/leg

$(LEG):
	CC=gcc make -C $(PEGDIR)

%.o : %.c markdown_peg.h
	$(CC) -c `pkg-config --cflags glib-2.0` $(CFLAGS) -o $@ $<

$(PROGRAM) : markdown.c $(OBJS)
	$(CC) `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` $(CFLAGS) -o $@ $(OBJS) $<

markdown_parser.c : markdown_parser.leg $(LEG) markdown_peg.h parsing_functions.c utility_functions.c
	$(LEG) -o $@ $<

.PHONY: clean test

clean:
	rm -f markdown_parser.c $(PROGRAM) $(OBJS); \
	make -C $(PEGDIR) clean

distclean: clean
	make -C $(PEGDIR) spotless

test: $(PROGRAM)
	cd MarkdownTest_1.0.3; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy  --Flags="-c"

mmdtest: $(PROGRAM)
	cd MarkdownTest_1.0.3; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy --testdir=MultiMarkdownTests

latextest: $(PROGRAM)
	cd MarkdownTest_1.0.3; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=LaTeXTests --Flags="-t latex"

leak-check: $(PROGRAM)
	valgrind --leak-check=full ./multimarkdown TEST.markdown > TEST.html

