ALL : multimarkdown

VERSION=3.1b1

PROGRAM=multimarkdown

# If you used fink to install gib2-dev, it might have been compiled for
# the i386 architecture, causing an error.  Can try adding -arch i686 to 
# CFLAGS

CFLAGS ?= -Wall -O3 -ansi

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
	make -C $(PEGDIR) clean; \
	rm -rf mac_installer/Package_Root/usr/local/bin; \
	rm -rf mac_installer/Support_Root; \
	rm mac_installer/Resources/*.html; \
	rm windows_installer/README.txt; \
	rm windows_installer/multimarkdown.exe; \
	rm windows_installer/multimarkdown.xml.backup; \
	rm windows_installer/LICENSE.html; \
	rm -rf mac_installer/*.pkg

distclean: clean
	make -C $(PEGDIR) spotless

test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy  --Flags="-c"

mmdtest: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests

compattest: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy --testdir=CompatibilityTests --Flags="-c"

latextest: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=BeamerTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MemoirTests --Flags="-t latex" --ext=".tex"

xslttest: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MultiMarkdownTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=BeamerTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MemoirTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \

leak-check: $(PROGRAM)
	valgrind --leak-check=full ./multimarkdown TEST.markdown > TEST.html

win-installer:
	cp README.markdown windows_installer/README.txt
	./multimarkdown LICENSE > windows_installer/LICENSE.html
	zip -r windows_installer/MultiMarkdown-Windows-$(VERSION).zip windows_installer/MMD-windows*.exe -x windows_installer/MultiMarkdown*.zip

mac-installer: $(PROGRAM)
	mkdir -p mac_installer/Package_Root/usr/local/bin
	mkdir -p mac_installer/Support_Root/Library/Application\ Support
	cp multimarkdown scripts/mmd* mac_installer/Package_Root/usr/local/bin/
	./multimarkdown README > mac_installer/Resources/README.html
	./multimarkdown mac_installer/Resources/Welcome.txt > mac_installer/Resources/Welcome.html
	./multimarkdown LICENSE > mac_installer/Resources/License.html
	./multimarkdown mac_installer/Resources/Support_Welcome.txt > mac_installer/Resources/Support_Welcome.html
	git clone Support mac_installer/Support_Root/Library/Application\ Support/MultiMarkdown
	cd mac_installer; /Developer/usr/bin/packagemaker \
	--doc "Make Support Installer.pmdoc" \
	--title "MultiMarkdown Support Files" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.MMD-Support.pkg \
	--domain user \
	--out "MultiMarkdown-Support-Mac-$(VERSION).pkg" \
	--no-relocate; \
	/Developer/usr/bin/packagemaker \
	--doc "Make OS X Installer.pmdoc" \
	--title "MultiMarkdown" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.multimarkdown.pkg \
	--out "MultiMarkdown-Mac-$(VERSION).pkg";
	
#	cp -r Support mac_installer/Support_Root/Library/Application\ Support/MultiMarkdown

# Requires installation of the platypus command line tool to create
# a drag and drop application for Mac OS X

drop: 
	mkdir drag; rm -rf drag/*.app; \
	/usr/local/bin/platypus -D -a 'MMD to LaTeX' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2LaTeX' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2tex' 'drag/MMD2LaTeX.app'; \
	/usr/local/bin/platypus -D -a 'MMD to HTML' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2HTML' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd' 'drag/MMD2HTML.app'; \
	/usr/local/bin/platypus -D -a 'MMD to OPML' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2OPML' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2opml' 'drag/MMD2OPML.app'; \
	/usr/local/bin/platypus -D -a 'MMD to ODF' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2ODF' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2odf' 'drag/MMD2ODF.app'; 

docs: $(PROGRAM)
	cd documentation; \
	../Support/Utilities/mmd_merge.pl index.txt > manual.txt; \
	../multimarkdown manual.txt > ../manual/index.html; \
	../multimarkdown -b -t latex manual.txt; \
	latexmk manual.tex; \
	latexmk -c manual.tex; \
	mv manual.pdf ../manual/mmd-manual.pdf; \
	cd ../manual; git add mmd-manual.pdf index.html; \
	git commit -m "update manual"; git push origin gh-pages; \
	rm ../documentation/manual.t*;
