Title:				peg-multimarkdown User's Guide  
Author:				Fletcher T. Penney  
Base Header Level:	2  

# Introduction #

Markdown is a simple markup language used to convert plain text into HTML.

[MultiMarkdown] is a derivative of [Markdown] that adds new syntax features,
such as footnotes, tables, and metadata. Additionally, it offers mechanisms to
convert plain text into LaTeX in addition to HTML.

[peg-multimarkdown] is an implementation of MultiMarkdown derived from John
MacFarlane's [peg-markdown]. It makes use of a parsing expression grammar
(PEG), and is written in C. It should compile for most any (major) operating
system.


# Installation #

For now - assume that if you can't figure it out, it's not for you. If you can
compile the original peg-markdown, this should work just fine. There are two
major hurdles at the moment:

* You need to have `glib2` installed - John MacFarlane used this to make use
  of GStrings, which now means you need a big huge library in order to compile
  this one simple program. I will work on removing this dependency, but need
  an alternative means of using dynamic strings in C --- I am not a C expert,
  and any pointers would be appreciated!

* The original makefile didn't work for me on my Mac. I have tweaked the
  makefile for this project to compile on a Mac, but it probably won't compile
  on other architectures. Again - any help on resolving this would be
  appreciated.

My goal is for this project to be able to compiled as a standalone binary so
that I can distribute pre-compiled binaries for Mac and Windows users. Linux
users probably won't have any problem, and it should be relatively easy for it
to be ported to many of the package maintenance systems out there once it's
finished.


## Install glib2 ##

To use on Ubuntu, for example, be sure to have installed glib2:

	sudo apt-get install libglib2.0-dev

For other linux distributions, you will need to do something similar.

On Mac OS X, you can use fink:

	fink install glib2-shlibs glib2-dev

(I don't remember if I had to install both packages, or just glib2-shlibs)

For Windows, follow the instructions included in `peg-markdown`. It's not
particularly straightforward.


## Compile peg-multimarkdown ##

Either download the peg-multimarkdown source, or use git:

	git clone git://github.com/fletcher/peg-multimarkdown.git

Then, simply run `make` to compile the source. You can also run some test
commands to verify that everything is working properly. Of note, it is normal
to fail one test in the Markdown tests, but the others should pass.

	make
	make mmdtest
	make latextest

On Mac OS X, you need to have installed the Developer Tools.


# Usage #

Once installed, you simply do something like the following:

* `markdown file.txt` --- process text into HTML.

* `markdown -c file.txt` --- use a compatibility mode that emulates the
  original Markdown.

* `markdown -t latex file.txt` --- output the results as LaTeX instead of
  HTML. This can then be processed into a PDF if you have LaTeX installed.

* `markdown -t memoir file.txt` --- output as LaTeX designed for use with the
  `memoir` package

* `markdown -t beamer file.txt` --- output as LaTeX designed for use with the
  `beamer` package

* `markdown -h` --- display help and additional options.


# Why create another version of MultiMarkdown? #

* Maintaining a growing collection of nested regular expressions was going to
  become increasingly difficult. I don't plan on adding much (if any) in the
  way of new syntax features, but it was a mess.

* Performance on longer documents was poor. The nested perl regular
  expressions was slow, even on a relatively fast computer. Performance on
  something like an iPhone would probably have been miserable.

* The reliance on Perl made installation fairly complex on Windows. That
  didn't bother me too much, but it is a factor.

* Perl can't be run on an iPhone/iPad, and I would like to be able to have
  MultiMarkdown on an iOS device, and not just regular Markdown (which exists
  in C versions).

* I was interested in learning about PEG's and revisiting C programming.

* The syntax has been fairly stable, and it would be nice to be able to
  formalize is a bit better --- which happens by definition when using a PEG.

* I wanted to revisit the syntax and features and clean it up a bit.

* Did I mention how much faster this is? And that it could (eventually) run on
  an iPhone?


# What's different? #

## "Complete" documents vs. "snippets" ##

A "snippet" is a section of HTML (or LaTeX) that is not a complete, fully-formed document.  It doesn't contain the header information to make it a valid XML document.  It can't be compiled with LaTeX into a PDF without further commands.

For example:

	# This is a header #
	
	And a paragraph.

becomes the following HTML snippet:

	<h1 id="thisisaheader">This is a header</h1>
	
	<p>And a paragraph.</p>

and the following LaTeX snippet:

	\part{This is a header}
	\label{thisisaheader}
	
	
	And a paragraph.

It was not possible to create a LaTeX snippet with the original MultiMarkdown,
because it relied on having a complete XHTML document that was then converted
to LaTeX via an XSLT document (requiring a whole separate program). This was
powerful, but complicated.

Now, I have come full-circle. peg-multimarkdown will now output LaTeX
directly, without requiring XSLT. This allows the creation of LaTeX snippets,
or complete documents, as necessary.

To create a complete document, simply include metadata. You can include a
title, author, date, or whatever you like. If you don't want to include any
real metadata, including "format: complete" will still trigger a complete
document, just like it used to.

The old approach (even though it was hidden from most users) was a bit of a
kludge, and this should be more elegant, and more flexible.


## Creating LaTeX Documents ##

LaTeX documents are created a bit differently than under the old system. You
no longer have to use an XSLT file to convert from XHTML to LaTeX. You can go
straight from MultiMarkdown to LaTeX, which is faster and more flexible.

**NOTE:** You can still use the old approach if you like. It was slow, but
powerful and allowed for fairly detailed customizations in your output.

To create a complete LaTeX document, you can process your file as a snippet,
and then place it in a LaTeX template that you already have. Alternatively,
you can use metadata to trigger the creation of a complete document. You can
use the `LaTeX Input` metadata to insert a `\input{file}` command. You can
then store various template files in your texmf directory and call them with
metadata, or with embedded raw LaTeX commands in your document. For example:

	Latex input:		mmd-memoir-header
	Latex input:		mmd-memoir-layout
	Title:				Sample MultiMarkdown Document  
	Author:				Fletcher T. Penney  
	latex input:		mmd-memoir-frontmatter
	latex footer:		mmd-memoir-footer

This would include several template files in the order that you see. The
`LaTeX Footer` metadata inserts a template at the end of your document.

This system isn't quite as powerful as the XSLT approach, since it doesn't
alter the actual MultiMarkdown to LaTeX conversion process. But it is probably
much more familiar to LaTeX users who are accustomed to using `\input{}`
commands and doesn't require knowledge of XSLT programming.


I recommend checking out the default [LaTeX Support Files] that are available
on github. They are designed to serve as a starting point for your own needs.

[LaTeX Support Files]: https://github.com/fletcher/peg-multimarkdown-latex-support


## Footnotes ##

Footnotes work slightly differently than before. This is partially on purpose,
and partly out of necessity.  Specifically:

* Footnotes are anchored based on number, rather than the label used in the
  MMD source. This won't show a visible difference to the reader, but the
  XHTML source will be different.

* Footnotes can be used more than once. Each reference will link to the same
  numbered note, but the "return" link will only link to the first instance.

* Footnote "return" links are a separate paragraph after the footnote. This is
  due to the way peg-markdown works, and it's not worth the effort to me to
  change it. You can always use CSS to change the appearance however you like.

* Footnote numbers are surrounded by "[]" in the text.


## Headers ##

If you use closing hashes at the end of your header, there needs to be a preceding space:

This:

	### Header ###

not this:

	### Header###


# Acknowledgments #

Thanks to John MacFarlane for [peg-markdown]. Obviously, this derivative work
would not be possible without his work. Additionally, he was very gracious in
giving me some pointers when I was getting started with trying to modify his
software.

Thanks to John Gruber for the original [Markdown]. 'Nuff said.

And thanks to the many contributors and users of the original MultiMarkdown
that helped me refine the syntax and search out bugs.

[peg-markdown]: https://github.com/jgm/peg-markdown
[Markdown]: http://daringfireball.net/projects/markdown/
[MultiMarkdown]: http://fletcherpenney.net/multimarkdown/
[peg-multimarkdown]: https://github.com/fletcher/peg-multimarkdown
