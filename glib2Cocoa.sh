#!/bin/sh


# Change STR to STRING
perl -pi -e 's/\bSTR\b/STRING/g' *.c *.h


# Change link to Link
perl -pi -e 's/(\s)link\b/$1Link/g' *.c *.leg *.h
perl -pi -e 's/\(link\)/(Link)/g' *.c *.leg *.h

# Change output of print_tree for compatibility
perl -pi -e 's/0x%x:/%p:/g' markdown_lib.c
perl -pi -e 's/\(int\)elt/elt/g' markdown_lib.c	


# Tweak peg
perl -pi -e 's/int len/size_t len/g' peg-0.1.4/compile.c
perl -pi -e 's/%d\\t/%td\\t/g' peg-0.1.4/tree.c


# typecast strlen
perl -pi -e 's/\+ \(strlen\(yy/+ (int)(strlen(yy/g' markdown_parser.leg


# Change to imports
perl -pi -e 's/#include "utility_functions.c"/#import "utility_functions.c"/g' markdown_parser.leg
perl -pi -e 's/#include "parsing_functions.c"/#import "parsing_functions.c"/g' markdown_parser.leg



# convert code from glib2 based g_strings to Cocoa NSMutableString
perl -pi -e 's/GString/NSMutableString/g' *.c *.h *.leg

perl -pi -e 's/g_string_append_printf\((.*?),\s*(".*?")\s*\)/[$1 appendString:\@$2]/g' *.c *.leg

perl -pi -e 's/g_string_append_printf\((.*?),\s*(.*)\);/[$1 appendFormat:\@$2];/g' *.c *.leg

perl -pi -e 's/char \*/NSString */g' *.c *.h *.leg
