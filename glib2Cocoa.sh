#!/bin/sh


# Change STR to STRING
perl -pi -e 's/\bSTR\b/STRING/g' *.m *.h


# Change link to Link
perl -pi -e 's/(\s)link\b/$1Link/g' *.m *.leg *.h
perl -pi -e 's/\(link\)/(Link)/g' *.m *.leg *.h

# Change output of print_tree for compatibility
perl -pi -e 's/0x%x:/%p:/g' markdown_lib.m
perl -pi -e 's/\(int\)elt/elt/g' markdown_lib.m	


# Tweak peg
perl -pi -e 's/int len/size_t len/g' peg-0.1.4/compile.c
perl -pi -e 's/%d\\t/%td\\t/g' peg-0.1.4/tree.c


# typecast strlen
perl -pi -e 's/\+ \(strlen\(yy/+ (int)(strlen(yy/g' markdown_parser.leg


# Change to imports
perl -pi -e 's/#include "utility_functions.m"/#import "utility_functions.m"/g' markdown_parser.leg
perl -pi -e 's/#include "parsing_functions.m"/#import "parsing_functions.m"/g' markdown_parser.leg
perl -pi -e 's/#include <glib.h>/#import <Foundation\/Foundation.h>/g' *.
perl -pi -e 's/#include "markdown_peg.h"/#import "markdown_peg.h"/g' *.



# convert code from glib2 based g_strings to Cocoa NSMutableString
perl -pi -e 's/GString/NSMutableString/g' *.m *.h *.leg

perl -pi -e 's/g_string_append_printf\((.*?),\s*(".*?")\s*\)/[$1 appendString:\@$2]/g' *.m *.leg

perl -pi -e 's/g_string_append_printf\((.*?),\s*(.*)\);/[$1 appendFormat:\@$2];/g' *.m *.leg

perl -pi -e 's/char \*/NSString */g' *.m *.h *.leg
