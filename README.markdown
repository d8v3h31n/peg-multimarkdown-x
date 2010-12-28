Title:				peg-multimarkdown User's Guide  
Author:				Fletcher T. Penney  
Base Header Level:	2  

# Introduction #

[Markdown] is a simple markup language used to convert plain text into HTML.

[MultiMarkdown] is a derivative of Markdown that adds new syntax features,
such as footnotes, tables, and metadata. Additionally, it offers mechanisms to
convert plain text into LaTeX in addition to HTML.

[peg-multimarkdown] is an implementation of MultiMarkdown derived from John
MacFarlane's [peg-markdown]. It makes use of a parsing expression grammar
(PEG), and is written in C. It should compile for most any (major) operating
system.


# Installation #


## Mac OS X ##

On the Mac, you can choose from using an installer to install the program and
required library for you, or you can compile it yourself from scratch. If you
know what that means, follow the instructions below in the Linux section.
Otherwise, definitely go for the installer!

Go to the [downloads] page to get the installer.


## Windows ##

The easiest way to get peg-multimarkdown running on Windows is to download the
precompiled binary from the [downloads] page. Theres a bat file you can run
that will move the included dll files to the proper location
(`C:\WINDOWS\system32`). If there's an easy way to make a proper installer, I
am open to the idea, but only if it's pretty easy to do (preferably from the
command line!)

If you want to compile this yourself, you do it in the same way that you would
install peg-markdown for Windows. The instructions are on the
peg-multimarkdown
[wiki](https://github.com/fletcher/peg-multimarkdown/wiki/Building-for-Windows)
page. I was able to compile for Windows fairly easily using Ubuntu linux
following those instructions. I have not tried to actually compile on a
Windows machine.

(For those who are interested, I actually created an EC2 instance on Amazon
and installed the necessary software to compile. I'll try and write down the
exact steps, but it was pretty easy and probably cost me $0.02...)


## Linux ##

The easiest way is probably to compile it yourself. You need to have `glib2`
installed. For example, on ubuntu:

	sudo apt-get install libglib2.0-dev

For other linux distributions, you will need to do something similar.

On Mac OS X, please note that you will need to install the Developer Tools.
Then you can use [fink] or something similar:

	fink install glib2-shlibs glib2-dev

Once you have that installed, you can either download the source for
[peg-multimarkdown], or you can use git:

	git clone git://github.com/fletcher/peg-multimarkdown.git

Then, simply run `make` to compile the source. You can also run some test
commands to verify that everything is working properly. Of note, it is normal
to fail one test in the Markdown tests, but the others should pass.

	make
	make test
	make mmdtest
	make latextest

If you're on a Mac, and want to make your own installer, you can run `make
installer` if you use the `mac-installer` branch of the github project. Then
open the `Make OS X Installer` file and build the installer using the
`PackageMaker` application.


# Usage #

Once installed, you simply do something like the following:

* `multimarkdown file.txt` --- process text into HTML.

* `multimarkdown -c file.txt` --- use a compatibility mode that emulates the
  original Markdown.

* `multimarkdown -t latex file.txt` --- output the results as LaTeX instead of
  HTML. This can then be processed into a PDF if you have LaTeX installed. You
  can further specify the `LaTeX Mode` metadata to customize output for
  compatibility with `memoir` or `beamer` classes.

* `multimarkdown -h` --- display help and additional options.

* `multimarkdown -b *.txt` --- `-b` or `--batch` mode can process multiple
  files at once, converting `file.txt` to `file.html` or `file.tex` as
  directed. Using this feature, you can convert a directory of MultiMarkdown
  text files into HTML files, or LaTeX files with a single command without
  having to specify the output files manually. **CAUTION**: This will
  overwrite existing files with the `html` or `tex` extension, so use with
  caution.

**Note**: The Mac OS X installer installs `mmd` as an alias to `multimarkdown`
for convenience. You could do the same on Windows (or rename the binary). You
could do the same manually on linux.


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

A "snippet" is a section of HTML (or LaTeX) that is not a complete,
fully-formed document. It doesn't contain the header information to make it a
valid XML document. It can't be compiled with LaTeX into a PDF without further
commands.

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

To create a complete LaTeX document, you can process your file as a snippet,
and then place it in a LaTeX template that you already have. Alternatively,
you can use metadata to trigger the creation of a complete document. You can
use the `LaTeX Input` metadata to insert a `\input{file}` command. You can
then store various template files in your texmf directory and call them with
metadata, or with embedded raw LaTeX commands in your document. For example:

	LaTeX Input:		mmd-memoir-header
	LaTeX Input:		mmd-memoir-layout
	Title:				Sample MultiMarkdown Document  
	Author:				Fletcher T. Penney  
	LaTeX Mode:			memoir  
	LaTeX Input:		mmd-memoir-frontmatter
	LaTeX Footer:		mmd-memoir-footer

This would include several template files in the order that you see. The
`LaTeX Footer` metadata inserts a template at the end of your document. Note
that the order and placement of the `LaTeX Include` statements is important.

The `LaTeX Mode` metadata allows you to specify that MultiMarkdown should use
the `memoir` or `beamer` output format. This places subtle differences in the
output document for compatibility with those respective classes.

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

If you use closing hashes at the end of your header, there needs to be a
preceding space:

This:

	### Header ###

not this:

	### Header###

This is a difference from the behavior of Markdown and many of its other
derivatives. However, at the moment it's not a priority for me to change it.


## Raw HTML ##

Because the original MultiMarkdown processed the text document into XHTML
first, and then processed the entire XHTML document into LaTeX, it couldn't
tell the difference between raw HTML and HTML that was created from plaintext.
This version, however, uses the original plain text to create the LaTeX
document. This means that any raw HTML inside your MultiMarkdown document is
**not** converted into LaTeX.

The benefit of this is that you can embed one piece of the document in two
formats --- one for XHTML, and one for LaTeX:

	<blockquote>
	<p>Release early, release often!</p>
	<blockquote><p>Linus Torvalds</p></blockquote>
	</blockquote>
	
	<!-- \epigraph{Release early, release often!}{Linus Torvalds} -->

In this section, when the document is converted into XHTML, the `blockquote`
sections will be used as expected, and the `epigraph` will be ignored since it
is inside a comment. Conversely, when processed into LaTeX, the raw HTML will
be ignored, and the comment will be processed as raw text.

You shouldn't need to use this feature, but if you want to specify exactly how
a certain part of your document is processed into LaTeX, it's a neat trick.


## Processing MultiMarkdown inside HTML ##

In the original MultiMarkdown, you could use something like `<div markdown=1>`
to tell MultiMarkdown to process the text inside the div. In
peg-multimarkdown, you can use the command-line option `--process-html` to
process the text inside all raw HTML. For the moment, there is not way to
process the text inside only selected HTML elements.


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
[fink]: http://www.finkproject.org/
[downloads]: https://github.com/fletcher/peg-multimarkdown/downloads