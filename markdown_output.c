/**********************************************************************

  markdown_output.c - functions for printing Elements parsed by 
                      markdown_peg.
  (c) 2008 John MacFarlane (jgm at berkeley dot edu).
  
  portions Copyright (c) 2010-2011 Fletcher T. Penney

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License or the MIT
  license.  See LICENSE for details.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

 ***********************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
#include "markdown_peg.h"
#include "utility_functions.c"

static int extensions;
static int base_header_level = 1;
static char *latex_footer;
static int table_column = 0;
static char *table_alignment;
static char cell_type = 'd';
static int language = ENGLISH;
static bool html_footer = FALSE;

static void print_html_string(GString *out, char *str, bool obfuscate);
static void print_html_element_list(GString *out, element *list, bool obfuscate);
static void print_html_element(GString *out, element *elt, bool obfuscate);
static void print_latex_string(GString *out, char *str);
static void print_latex_element_list(GString *out, element *list);
static void print_latex_element(GString *out, element *elt);
static void print_groff_string(GString *out, char *str);
static void print_groff_mm_element_list(GString *out, element *list);
static void print_groff_mm_element(GString *out, element *elt, int count);


/* MultiMarkdown Routines */
static void print_html_header(GString *out, element *elt, bool obfuscate);
static void print_html_footer(GString *out, bool obfuscate);

static void print_latex_header(GString *out, element *elt);
static void print_latex_footer(GString *out);

static void print_memoir_element_list(GString *out, element *list);
static void print_memoir_element(GString *out, element *elt);

static void print_beamer_element_list(GString *out, element *list);
static void print_beamer_element(GString *out, element *elt);

static void print_opml_string(GString *out, char *str);
static void print_opml_element_list(GString *out, element *list);
static void print_opml_element(GString *out, element *elt);

element * print_html_headingsection(GString *out, element *list, bool obfuscate);

static bool list_contains_key(element *list, int key);
static bool is_html_complete_doc(element *meta);
static int find_latex_mode(int format, element *list);
element * metadata_for_key(char *key, element *list);
char * metavalue_for_key(char *key, element *list);

element * element_for_attribute(char *querystring, element *list);
char * dimension_for_attribute(char *querystring, element *list);

/**********************************************************************

  Utility functions for printing

 ***********************************************************************/

static int padded = 2;      /* Number of newlines after last output.
                               Starts at 2 so no newlines are needed at start.
                               */

static GSList *endnotes = NULL; /* List of endnotes to print after main content. */
static int notenumber = 0;  /* Number of footnote. */

/* pad - add newlines if needed */
static void pad(GString *out, int num) {
    while (num-- > padded)
        g_string_append_printf(out, "\n");;
    padded = num;
}

/**********************************************************************

  Functions for printing Elements as HTML

 ***********************************************************************/

/* print_html_string - print string, escaping for HTML  
 * If obfuscate selected, convert characters to hex or decimal entities at random */
static void print_html_string(GString *out, char *str, bool obfuscate) {
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
        default:
            if (obfuscate) {
                if (rand() % 2 == 0)
                    g_string_append_printf(out, "&#%d;", (int) *str);
                else
                    g_string_append_printf(out, "&#x%x;", (unsigned int) *str);
            }
            else
                g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_html_element_list - print a list of elements as HTML */
static void print_html_element_list(GString *out, element *list, bool obfuscate) {
    while (list != NULL) {
        if (list->key == HEADINGSECTION) {
            list = print_html_headingsection(out, list, obfuscate);
        } else {
            print_html_element(out, list, obfuscate);
            list = list->next;
        }
    }
}

/* add_endnote - add an endnote to global endnotes list. */
static void add_endnote(element *elt) {
    endnotes = g_slist_prepend(endnotes, elt);
}

/* print_html_element - print an element as HTML */
static void print_html_element(GString *out, element *elt, bool obfuscate) {
    int lev;
    char *label;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINEBREAK:
        g_string_append_printf(out, "<br/>\n");
        break;
    case STR:
        print_html_string(out, elt->contents.str, obfuscate);
        break;
    case ELLIPSIS:
        localize_typography(out, ELLIP, language, HTMLOUT);
        break;
    case EMDASH:
        localize_typography(out, MDASH, language, HTMLOUT);
        break;
    case ENDASH:
        localize_typography(out, NDASH, language, HTMLOUT);
        break;
    case APOSTROPHE:
        localize_typography(out, APOS, language, HTMLOUT);
        break;
    case SINGLEQUOTED:
        localize_typography(out, LSQUOTE, language, HTMLOUT);
        print_html_element_list(out, elt->children, obfuscate);
        localize_typography(out, RSQUOTE, language, HTMLOUT);
        break;
    case DOUBLEQUOTED:
        localize_typography(out, LDQUOTE, language, HTMLOUT);
        print_html_element_list(out, elt->children, obfuscate);
        localize_typography(out, RDQUOTE, language, HTMLOUT);
        break;
    case CODE:
        g_string_append_printf(out, "<code>");
        print_html_string(out, elt->contents.str, obfuscate);
        g_string_append_printf(out, "</code>");
        break;
    case HTML:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINK:
        if (strstr(elt->contents.link->url, "mailto:") == elt->contents.link->url)
            obfuscate = true;  /* obfuscate mailto: links */
        g_string_append_printf(out, "<a href=\"");
        print_html_string(out, elt->contents.link->url, obfuscate);
        g_string_append_printf(out, "\"");
        if (strlen(elt->contents.link->title) > 0) {
            g_string_append_printf(out, " title=\"");
            print_html_string(out, elt->contents.link->title, obfuscate);
            g_string_append_printf(out, "\"");
        }
        print_html_element_list(out, elt->contents.link->attr, obfuscate);
        g_string_append_printf(out, ">");
        print_html_element_list(out, elt->contents.link->label, obfuscate);
        g_string_append_printf(out, "</a>");
        break;
    case IMAGE:
        g_string_append_printf(out, "<img src=\"");
        print_html_string(out, elt->contents.link->url, obfuscate);
        if ( extension(EXT_COMPATIBILITY) ) {
            g_string_append_printf(out, "\"");
        } else {
            g_string_append_printf(out, "\" id=\"%s\"",elt->contents.link->identifier);
        }
        g_string_append_printf(out, " alt=\"");
        print_raw_element_list(out, elt->contents.link->label);
        g_string_append_printf(out, "\"");
        if (strlen(elt->contents.link->title) > 0) {
            g_string_append_printf(out, " title=\"");
            print_html_string(out, elt->contents.link->title, obfuscate);
            g_string_append_printf(out, "\"");
        }
        print_html_element_list(out, elt->contents.link->attr, obfuscate);
        g_string_append_printf(out, " />");
        break;
    case EMPH:
        g_string_append_printf(out, "<em>");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</em>");
        break;
    case STRONG:
        g_string_append_printf(out, "<strong>");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</strong>");
        break;
    case LIST:
        print_html_element_list(out, elt->children, obfuscate);
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
        if (lev > 6)
            lev = 6;
        pad(out, 2);
        if ( extension(EXT_COMPATIBILITY) ) {
            /* Use regular Markdown header format */
            g_string_append_printf(out, "<h%1d>", lev);
            print_html_element_list(out, elt->children, obfuscate);
        } else if (elt->children->key == AUTOLABEL) {
            /* generate a label for each header (MMD)*/
            g_string_append_printf(out, "<h%d id=\"%s\">", lev,elt->children->contents.str);
            print_html_element_list(out, elt->children->next, obfuscate);
        } else {
            label = label_from_element_list(elt->children, obfuscate);
            g_string_append_printf(out, "<h%d id=\"%s\">", lev, label);
            print_html_element_list(out, elt->children, obfuscate);
            free(label);
        }
        g_string_append_printf(out, "</h%1d>", lev);
        padded = 0;
        break;
    case PLAIN:
        pad(out, 1);
        print_html_element_list(out, elt->children, obfuscate);
        padded = 0;
        break;
    case PARA:
        pad(out, 2);
        g_string_append_printf(out, "<p>");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</p>");
        padded = 0;
        break;
    case HRULE:
        pad(out, 2);
        g_string_append_printf(out, "<hr />");
        padded = 0;
        break;
    case HTMLBLOCK:
        pad(out, 2);
        g_string_append_printf(out, "%s", elt->contents.str);
        padded = 0;
        break;
    case VERBATIM:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<pre><code>");
        print_html_string(out, elt->contents.str, obfuscate);
        g_string_append_printf(out, "%s", "</code></pre>");
        padded = 0;
        break;
    case BULLETLIST:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<ul>");
        padded = 0;
        print_html_element_list(out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "%s", "</ul>");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<ol>");
        padded = 0;
        print_html_element_list(out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "</ol>");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, "<li>");
        padded = 2;
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</li>");
        padded = 0;
        break;
    case BLOCKQUOTE:
        pad(out, 2);
        g_string_append_printf(out, "<blockquote>\n");
        padded = 2;
        print_html_element_list(out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "</blockquote>");
        padded = 0;
        break;
    case REFERENCE:
        /* Nonprinting */
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            if ( (elt->children->contents.str == 0) ){
                /* The referenced note has not been used before */
                add_endnote(elt->children);
                ++notenumber;
                char buf[5];
                sprintf(buf,"%d",notenumber);
                /* Assign footnote number for future use */
                elt->children->contents.str = strdup(buf);
                if (elt->children->key == GLOSSARYTERM) {
                    g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote glossary\">[%d]</a>",
                                notenumber, notenumber, notenumber);
                } else {
                    g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote\">[%d]</a>",
                                notenumber, notenumber, notenumber);
                }
            } else {
                /* The referenced note has already been used */
                g_string_append_printf(out, "<a href=\"#fn:%s\" title=\"see footnote\" class=\"footnote\">[%s]</a>",
                    elt->children->contents.str, elt->children->contents.str);
            }
        }
        elt->children = NULL;
        break;
    case GLOSSARY:
        /* Shouldn't do anything */
        break;
    case GLOSSARYTERM:
        g_string_append_printf(out,"<span class=\"glossary name\">");
        print_html_string(out, elt->children->contents.str, obfuscate);
        g_string_append_printf(out, "</span>");
        if ((elt->next != NULL) && (elt->next->key == GLOSSARYSORTKEY) ) {
            g_string_append_printf(out, "<span class=\"glossary sort\" style=\"display:none\">");
            print_html_string(out, elt->next->contents.str, obfuscate);
            g_string_append_printf(out, "</span>");
        }
        g_string_append_printf(out, ": ");
        break;
    case GLOSSARYSORTKEY:
        break;
    case NOCITATION:
    case CITATION:
        if ((elt->children != NULL) && (elt->children->key == LOCATOR)) {
            GString *temp = g_string_new("");
            print_html_element(temp,elt->children,obfuscate);
            label = strdup(temp->str);
            g_string_free(temp,true);
            elt->children = elt->children->next;
        } else {
            label = NULL;
        }
        if (strncmp(elt->contents.str,"[#",2) == 0) {
            /* reference specified externally */
            if ( elt->key == NOCITATION ) {
                g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\"/>", elt->contents.str);
            } else {
                g_string_append_printf(out, "<span class=\"externalcitation\">");
                if (label != NULL) g_string_append_printf(out, "[%s]", label);
                g_string_append_printf(out, "%s",elt->contents.str);
                g_string_append_printf(out, "</span>");
            }
        } else {
            /* reference specified within the MMD document */
            if (elt->children->contents.str == NULL) {
                elt->children->key = CITATION;
                add_endnote(elt->children);
                ++notenumber;
                char buf[5];
                sprintf(buf,"%d",notenumber);
                
                elt->children->contents.str = strdup(buf);
            }
            if (label != NULL) {
                if ( elt->key == NOCITATION ) {
                    g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\">",
                        elt->children->contents.str);
                } else {
                    g_string_append_printf(out, "<a class=\"citation\" href=\"#fn:%s\" title=\"Jump to citation\">[<span class=\"locator\">%s</span>, %s]",
                        elt->children->contents.str, label, elt->children->contents.str);
                }
                elt->children = NULL;               
            } else {
                g_string_append_printf(out, "<a class=\"citation\" href=\"#fn:%s\" title=\"Jump to citation\">[%s]",
                    elt->children->contents.str, elt->children->contents.str);
                elt->children = NULL;
            }
            g_string_append_printf(out, "<span class=\"citekey\" style=\"display:none\">%s</span>", elt->contents.str);
            if (label != NULL) {
                if ( elt->key == NOCITATION ) {
                    g_string_append_printf(out,"</span>");
                } else {
                    g_string_append_printf(out,"</a>");
                }
            } else {
                g_string_append_printf(out,"</a>");
            }
        }
        free(label);
        break;
    case LOCATOR:
        print_html_element_list(out, elt->children, obfuscate);
        break;
    case DEFLIST:
        pad(out,1);
        padded = 1;
        g_string_append_printf(out, "<dl>\n");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</dl>\n");
        padded = 0;
        break;
    case TERM:
        pad(out,1);
        g_string_append_printf(out, "<dt>");
        print_html_string(out, elt->contents.str, obfuscate);
        g_string_append_printf(out, "</dt>\n");
        padded = 1;
        break;
    case DEFINITION:
        pad(out,1);
        padded = 1;
        g_string_append_printf(out, "<dd>");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</dd>\n");
        padded = 0;
        break;
    case METADATA:
        /* Metadata is present, so this should be a "complete" document */
        html_footer = is_html_complete_doc(elt);
        if (html_footer) {
            print_html_header(out, elt, obfuscate);
        } else {
            print_html_element_list(out, elt->children, obfuscate);
        }
        break;
    case METAKEY:
        if (strcmp(elt->contents.str, "title") == 0) {
            g_string_append_printf(out, "\t<title>");
            print_html_element(out, elt->children, obfuscate);
            g_string_append_printf(out, "</title>\n");
        } else if (strcmp(elt->contents.str, "css") == 0) {
            g_string_append_printf(out, "\t<link type=\"text/css\" rel=\"stylesheet\" href=\"");
            print_html_element(out, elt->children, obfuscate);
            g_string_append_printf(out, "\"/>\n");
        } else if (strcmp(elt->contents.str, "xhtmlheader") == 0) {
            print_raw_element(out, elt->children);
            g_string_append_printf(out, "\n");
        } else if (strcmp(elt->contents.str, "baseheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "quoteslanguage") == 0) {
            label = label_from_element_list(elt->children, 0);
            if (strcmp(label, "dutch") == 0) { language = DUTCH; } else 
            if (strcmp(label, "german") == 0) { language = GERMAN; } else 
            if (strcmp(label, "germanguillemets") == 0) { language = GERMANGUILL; } else 
            if (strcmp(label, "french") == 0) { language = FRENCH; } else 
            if (strcmp(label, "swedish") == 0) { language = SWEDISH; }
            free(label);
       } else {
            g_string_append_printf(out, "\t<meta name=\"");
            print_html_string(out, elt->contents.str, obfuscate);
            g_string_append_printf(out, "\" content=\"");
            print_html_element(out, elt->children, obfuscate);
            g_string_append_printf(out, "\"/>\n");
        }
        break;
    case METAVALUE:
        print_html_string(out, elt->contents.str, obfuscate);
        break;
    case FOOTER:
        break;
    case HEADINGSECTION:
        print_html_element_list(out, elt->children, obfuscate);
        break;
    case TABLE:
        g_string_append_printf(out, "\n\n<table>\n");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</table>\n");
        break;
    case TABLESEPARATOR:
        table_alignment = elt->contents.str;
        break;
    case TABLECAPTION:
        if (elt->children->key == TABLELABEL) {
            label = label_from_element_list(elt->children->children,obfuscate);
        } else {
            label = label_from_element_list(elt->children,obfuscate);
        }
        g_string_append_printf(out, "<caption id=\"%s\">", label);
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</caption>\n");
        free(label);
        break;
    case TABLELABEL:
        break;
    case TABLEHEAD:
        /* print column alignment for XSLT processing if needed */
        for (table_column=0;table_column<strlen(table_alignment);table_column++) {
           if ( strncmp(&table_alignment[table_column],"r",1) == 0) {
                g_string_append_printf(out, "<col align=\"right\"/>\n");
            } else if ( strncmp(&table_alignment[table_column],"c",1) == 0) {
                g_string_append_printf(out, "<col align=\"center\"/>\n");
            } else {
                g_string_append_printf(out, "<col align=\"left\"/>\n");
            }
        }
        cell_type = 'h';
        g_string_append_printf(out, "\n<thead>\n");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</thead>\n");
        cell_type = 'd';
        break;
    case TABLEBODY:
        g_string_append_printf(out, "\n<tbody>\n");
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</tbody>\n");
        break;
    case TABLEROW:
        g_string_append_printf(out, "<tr>\n");
        table_column = 0;
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</tr>\n");
        break;
    case TABLECELL:
        if ( strncmp(&table_alignment[table_column],"r",1) == 0) {
            g_string_append_printf(out, "\t<t%c align=\"right\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"c",1) == 0) {
            g_string_append_printf(out, "\t<t%c align=\"center\"", cell_type);
        } else {
            g_string_append_printf(out, "\t<t%c align=\"left\"", cell_type);
        }
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, " colspan=\"%d\"",(int)strlen(elt->children->contents.str)+1);
        }
        g_string_append_printf(out, ">");
        padded = 2;
        print_html_element_list(out, elt->children, obfuscate);
        g_string_append_printf(out, "</t%c>\n", cell_type);
        table_column++;
        break;
    case CELLSPAN:
        break;
    case ATTRKEY:
        g_string_append_printf(out, " %s=\"%s\"", elt->contents.str,
            elt->children->contents.str);
        break;
    case MATHSPAN:
        if ( elt->contents.str[strlen(elt->contents.str)-1] == ']') {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "<span class=\"math\">%s\\]</span>", elt->contents.str);
        } else {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "<span class=\"math\">%s\\)</span>", elt->contents.str);
        }
        break;
    default: 
        fprintf(stderr, "print_html_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

static void print_html_endnotes(GString *out) {
    int counter = 0;
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    g_string_append_printf(out, "<div class=\"footnotes\">\n<hr />\n<ol>");
    while (note != NULL) {
        note_elt = note->data;
        counter++;
        pad(out, 1);
        if (note_elt->key == CITATION) {
            g_string_append_printf(out, "<li id=\"fn:%s\" class=\"citation\"><span class=\"citekey\" style=\"display:none\">", note_elt->contents.str);
            element *temp = note_elt;
            while ( temp != NULL ) {
                if (temp->key == NOTELABEL)
                    print_html_string(out, temp->contents.str, 0);
                temp = temp->next;
            }
            g_string_append_printf(out, "</span>");
            padded = 2;
            print_html_element_list(out, note_elt->children, false);
            pad(out, 1);
            g_string_append_printf(out, "</li>");
        } else {
            g_string_append_printf(out, "<li id=\"fn:%d\">\n", counter);
            padded = 2;
            print_html_element_list(out, note_elt, false);
            g_string_append_printf(out, " <a href=\"#fnref:%d\" title=\"return to article\" class=\"reversefootnote\">&#160;&#8617;</a>", counter);
            pad(out, 1);
            g_string_append_printf(out, "</li>");
        }
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "</ol>\n</div>\n");

    g_slist_free(endnotes);
}

/**********************************************************************

  Functions for printing Elements as LaTeX

 ***********************************************************************/

/* print_latex_string - print string, escaping for LaTeX */
static void print_latex_string(GString *out, char *str) {
	char *tmp;
    while (*str != '\0') {
        switch (*str) {
          case '{': case '}': case '$': case '%':
          case '&': case '_': case '#':
            g_string_append_printf(out, "\\%c", *str);
            break;
        case '^':
            g_string_append_printf(out, "\\^{}");
            break;
        case '\\':
            g_string_append_printf(out, "$\\backslash$");
            break;
        case '~':
            g_string_append_printf(out, "\\ensuremath{\\sim}");
            break;
        case '|':
            g_string_append_printf(out, "\\textbar{}");
            break;
        case '<':
            g_string_append_printf(out, "$<$");
            break;
        case '>':
            g_string_append_printf(out, "$>$");
            break;
        case '/':
            g_string_append_printf(out, "\\slash ");
            break;
        case '\n':
            tmp = str;
            tmp--;
            if (*tmp == ' ') {
                tmp--;
                if (*tmp == ' ') {
                    g_string_append_printf(out, "\\\\\n");
                } else {
                    g_string_append_printf(out, "\n");
                }
            } else {
                g_string_append_printf(out, "\n");
            }
            break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}

static void print_latex_endnotes(GString *out) {
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    pad(out,2);
    g_string_append_printf(out, "\\begin{thebibliography}{0}");
    while (note != NULL) {
        note_elt = note->data;
        pad(out, 1);
        g_string_append_printf(out, "\\bibitem{%s}\n", note_elt->contents.str);
        padded=2;
        print_latex_element_list(out, note_elt);
        pad(out, 1);
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "\\end{thebibliography}\n");
    padded = 1;
    g_slist_free(endnotes);
}

/* print_latex_element_list - print a list of elements as LaTeX */
static void print_latex_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_latex_element(out, list);
        list = list->next;
    }
}

/* print_latex_element - print an element as LaTeX */
static void print_latex_element(GString *out, element *elt) {
    int lev;
    char *label;
    char *height;
    char *width;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINEBREAK:
        g_string_append_printf(out, "\\\\\n");
        break;
    case STR:
        print_latex_string(out, elt->contents.str);
        break;
    case ELLIPSIS:
        localize_typography(out, ELLIP, language, LATEXOUT);
        break;
    case EMDASH: 
        localize_typography(out, MDASH, language, LATEXOUT);
        break;
    case ENDASH: 
        localize_typography(out, NDASH, language, LATEXOUT);
        break;
    case APOSTROPHE:
        localize_typography(out, APOS, language, LATEXOUT);
        break;
    case SINGLEQUOTED:
        localize_typography(out, LSQUOTE, language, LATEXOUT);
        print_latex_element_list(out, elt->children);
        localize_typography(out, RSQUOTE, language, LATEXOUT);
        break;
    case DOUBLEQUOTED:
        localize_typography(out, LDQUOTE, language, LATEXOUT);
        print_latex_element_list(out, elt->children);
        localize_typography(out, RDQUOTE, language, LATEXOUT);
        break;
    case CODE:
        g_string_append_printf(out, "\\texttt{");
        print_latex_string(out, elt->contents.str);
        g_string_append_printf(out, "}");
        break;
    case HTML:
        /* don't print HTML */
        /* but do print HTML comments for raw LaTeX */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s", &elt->contents.str[4]);
        }
        break;
    case LINK:
        if (elt->contents.link->url[0] == '#') {
            /* This is a link to anchor within document */
            label = label_from_string(elt->contents.link->url,0);
            if (elt->contents.link->label != NULL) {
                    print_latex_element_list(out, elt->contents.link->label);
                g_string_append_printf(out, " (\\autoref\{%s})", label);             
            } else {
                g_string_append_printf(out, "\\autoref\{%s}", label);
            }
            free(label);
        } else if ( (elt->contents.link->label != NULL) &&
                ( strcmp(elt->contents.link->label->contents.str, 
                elt->contents.link->url) == 0 )) {
            /* This is a <link> */
            g_string_append_printf(out, "\\href{%s}{", elt->contents.link->url);
            print_latex_string(out, elt->contents.link->url);
            g_string_append_printf(out, "}");
        } else if ( (elt->contents.link->label != NULL) && 
                ( strcmp(&elt->contents.link->url[7], 
                elt->contents.link->label->contents.str) == 0 )) {
            /* This is a <mailto> */
            g_string_append_printf(out, "\\href{%s}{%s}", elt->contents.link->url, &elt->contents.link->url[7]);
        } else {
            /* This is a [text](link) */
            g_string_append_printf(out, "\\href{%s}{", elt->contents.link->url);
            print_latex_element_list(out, elt->contents.link->label);
            g_string_append_printf(out, "}\\footnote{\\href{%s}{", elt->contents.link->url);
            print_latex_string(out, elt->contents.link->url);
            g_string_append_printf(out, "}}");
        }
        break;
    case IMAGE:
        /* Figure if we have height, width, neither */
        height = dimension_for_attribute("height", elt->contents.link->attr);
        width = dimension_for_attribute("width", elt->contents.link->attr);
        g_string_append_printf(out, "\\begin{figure}[htbp]\n\\centering\n\\includegraphics[");
        if ((height == NULL) && (width == NULL)) {
            /* No dimensions given */
            g_string_append_printf(out,"keepaspectratio,width=\\textwidth,height=0.75\\textheight");
        } else {
            /* at least one dimension given */
            if ((height != NULL) && (width != NULL)) {
                
            } else {
                g_string_append_printf(out, "keepaspectratio,");
            }
            if (width != NULL) {
                g_string_append_printf(out,"width=%s,", width);
            } else {
                g_string_append_printf(out, "width=\\textwidth,");
            }
            if (height != NULL) {
                g_string_append_printf(out,"height=%s",height);
            } else {
                g_string_append_printf(out, "height=0.75\\textheight");
            }
        }

        g_string_append_printf(out, "]{%s}\n", elt->contents.link->url);
        if (strlen(elt->contents.link->title) > 0) {
            g_string_append_printf(out, "\\caption{");
            print_latex_string(out, elt->contents.link->title);
            g_string_append_printf(out, "}\n");
        }
        g_string_append_printf(out, "\\label{%s}\n", elt->contents.link->identifier);
        g_string_append_printf(out,"\\end{figure}\n");
        free(height);
        free(width);
        break;
    case EMPH:
        g_string_append_printf(out, "\\emph{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}");
        break;
    case STRONG:
        g_string_append_printf(out, "\\textbf{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}");
        break;
    case LIST:
        print_latex_element_list(out, elt->children);
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        pad(out, 2);
        lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
        switch (lev) {
            case 1:
                g_string_append_printf(out, "\\part{");
                break;
            case 2:
                g_string_append_printf(out, "\\chapter{");
                break;
            case 3:
                g_string_append_printf(out, "\\section{");
                break;
            case 4:
                g_string_append_printf(out, "\\subsection{");
                break;
            case 5:
                g_string_append_printf(out, "\\subsubsection{");
                break;
            case 6:
                g_string_append_printf(out, "\\paragraph{");
                break;
            case 7:
                g_string_append_printf(out, "\\subparagraph{");
                break;
            default:
                g_string_append_printf(out, "\\noindent\\textbf{");
                break;
        }
        /* generate a label for each header (MMD)*/
        if (elt->children->key == AUTOLABEL) {
            label = label_from_string(elt->children->contents.str,0);
            print_latex_element_list(out, elt->children->next);
            g_string_append_printf(out, "}\n\\label{");
            g_string_append_printf(out, "%s", label);
            free(label);
        } else {
            label = label_from_element_list(elt->children,0);
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n\\label{");
            g_string_append_printf(out, "%s", label);
            free(label);
        }
        g_string_append_printf(out, "}\n");
        padded = 1;
        break;
    case PLAIN:
        pad(out, 1);
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case PARA:
        pad(out, 2);
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case HRULE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{center}\\rule{3in}{0.4pt}\\end{center}\n");
        padded = 0;
        break;
    case HTMLBLOCK:
        /* don't print HTML block */
        /* but do print HTML comments for raw LaTeX */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            pad(out, 2);
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s", &elt->contents.str[4]);
            padded = 0;
        }
        break;
    case VERBATIM:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{verbatim}\n");
        print_raw_element(out, elt);
        g_string_append_printf(out, "\\end{verbatim}\n");
        padded = 0;
        break;
    case BULLETLIST:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{itemize}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n\\end{itemize}");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{enumerate}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, "\\end{enumerate}");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, "\\item ");
        padded = 2;
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n");
        break;
    case BLOCKQUOTE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{quote}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, "\\end{quote}");
        padded = 0;
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            if (elt->children->key == GLOSSARYTERM) {
                g_string_append_printf(out, "\\newglossaryentry{%s}{", elt->children->children->contents.str);
                padded = 2;
                if (elt->children->next->key == GLOSSARYSORTKEY) {
                    g_string_append_printf(out, "sort={");
                    print_latex_string(out, elt->children->next->contents.str);
                    g_string_append_printf(out, "},");
                }
                print_latex_element_list(out, elt->children);
                g_string_append_printf(out, "}}\\glsadd{%s}", elt->children->children->contents.str);
                padded = 0;
            } else {
                g_string_append_printf(out, "\\footnote{");
                padded = 2;
                print_latex_element_list(out, elt->children);
                g_string_append_printf(out, "}");
                padded = 0;
            }
            elt->children = NULL;
        }
        break;
    case GLOSSARY:
        /* This shouldn't do anything */
        break;
    case GLOSSARYTERM:
        g_string_append_printf(out, "name={");
        print_latex_string(out, elt->children->contents.str);
        g_string_append_printf(out, "},description={");
        break;
    case GLOSSARYSORTKEY:
        break;
    case REFERENCE:
        /* Nonprinting */
        break;
    case NOCITATION:
    case CITATION:
        if (strncmp(elt->contents.str,"[#",2) == 0) {
            /* This should be used as a bibtex citation key after trimming */
            elt->contents.str[strlen(elt->contents.str)-1] = '\0';
            if (elt->key == NOCITATION ) {
                g_string_append_printf(out, "~\\nocite{%s}", &elt->contents.str[2]);
            } else {
                if ((elt->children != NULL) && (elt->children->key == LOCATOR)) {
                    g_string_append_printf(out, "~\\cite[");
                    print_latex_element(out,elt->children);
                    g_string_append_printf(out, "]{%s}",&elt->contents.str[2]);
                } else {
                    g_string_append_printf(out, "~\\cite{%s}", &elt->contents.str[2]);
                }
            }
        } else {
            /* This citation was specified in the document itself */
            if (elt->key == NOCITATION ) {
                g_string_append_printf(out, "~\\nocite{%s}", elt->contents.str);
                element *temp;
                temp = elt->children;
                elt->children = temp->next;
                free_element(temp);
            } else {
                if ((elt->children != NULL) && (elt->children->key == LOCATOR)){
                    g_string_append_printf(out, "~\\cite[");
                    print_latex_element(out,elt->children);
                    g_string_append_printf(out, "]{%s}",elt->contents.str);
                    element *temp;
                    temp = elt->children;
                    elt->children = temp->next;
                    free_element(temp);
                } else {
                    g_string_append_printf(out, "~\\cite{%s}", elt->contents.str);
                }
            }
            if (elt->children->contents.str == NULL) {
                elt->children->contents.str = strdup(elt->contents.str);
                add_endnote(elt->children);
            }
            elt->children = NULL;
        }
        break;
    case LOCATOR:
        print_latex_element_list(out, elt->children);
        break;
    case DEFLIST:
        g_string_append_printf(out, "\\begin{description}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out,1);
        g_string_append_printf(out, "\\end{description}");
        padded = 0;
        break;
    case TERM:
        pad(out,2);
        g_string_append_printf(out, "\\item[%s]", elt->contents.str);
        padded = 0;
        break;
    case DEFINITION:
        pad(out,2);
        padded = 2;
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case METADATA:
        /* Metadata is present, so this should be a "complete" document */
        print_latex_header(out, elt);
        break;
    case METAKEY:
        if (strcmp(elt->contents.str, "title") == 0) {
            g_string_append_printf(out, "\\def\\mytitle{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "author") == 0) {
            g_string_append_printf(out, "\\def\\myauthor{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "date") == 0) {
            g_string_append_printf(out, "\\def\\mydate{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "baseheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "latexinput") == 0) {
            g_string_append_printf(out, "\\input{%s}\n", elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "latexfooter") == 0) {
            latex_footer = elt->children->contents.str;
        } else if (strcmp(elt->contents.str, "bibtex") == 0) {
            g_string_append_printf(out, "\\def\\bibliocommand{\\bibliography{%s}}\n",elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "xhtmlheader") == 0) {
        } else if (strcmp(elt->contents.str, "css") == 0) {
        } else if (strcmp(elt->contents.str, "quoteslanguage") == 0) {
            label = label_from_element_list(elt->children, 0);
            if (strcmp(label, "dutch") == 0) { language = DUTCH; } else 
            if (strcmp(label, "german") == 0) { language = GERMAN; } else 
            if (strcmp(label, "germanguillemets") == 0) { language = GERMANGUILL; } else 
            if (strcmp(label, "french") == 0) { language = FRENCH; } else 
            if (strcmp(label, "swedish") == 0) { language = SWEDISH; }
            free(label);
        } else {
            g_string_append_printf(out, "\\def\\");
            print_latex_string(out, elt->contents.str);
            g_string_append_printf(out, "{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        }
        break;
    case METAVALUE:
        print_latex_string(out, elt->contents.str);
        break;
    case FOOTER:
        print_latex_endnotes(out);
        print_latex_footer(out);
        break;
    case HEADINGSECTION:
        print_latex_element_list(out, elt->children);
        break;
    case TABLE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{table}[htbp]\n\\begin{minipage}{\\linewidth}\n\\setlength{\\tymax}{0.5\\linewidth}\n\\centering\n\\small\n");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n\\end{tabular}\n\\end{minipage}\n\\end{table}\n");
        padded = 0;
        break;
    case TABLESEPARATOR:
        g_string_append_printf(out, "\\begin{tabular}{@{}%s@{}} \\toprule\n", elt->contents.str);
        break;
    case TABLECAPTION:
        if (elt->children->key == TABLELABEL) {
            label = label_from_element_list(elt->children->children,0);
        } else {
            label = label_from_element_list(elt->children,0);
        }
        g_string_append_printf(out, "\\caption{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}\n\\label{%s}\n",label);
        free(label);
        break;
    case TABLELABEL:
        break;
    case TABLEHEAD:
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\\midrule\n");
        break;
    case TABLEBODY:
        print_latex_element_list(out, elt->children);
        if ( ( elt->next != NULL ) && (elt->next->key == TABLEBODY) ) {
            g_string_append_printf(out, "\n\\midrule\n");
        } else {
            g_string_append_printf(out, "\n\\bottomrule\n");
        }
        break;
    case TABLEROW:
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\\\\\n");
        break;
    case TABLECELL:
        padded = 2;
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, "\\multicolumn{%d}{c}{", (int)strlen(elt->children->contents.str)+1);
        }
        print_latex_element_list(out, elt->children);
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, "}");
        }
        if (elt->next != NULL) {
            g_string_append_printf(out, "&");
        }
        break;
    case CELLSPAN:
        break;
    case ATTRKEY:
        g_string_append_printf(out, " %s=\"%s\"", elt->contents.str,
            elt->children->contents.str);
        break;
    case MATHSPAN:
        if ( elt->contents.str[strlen(elt->contents.str)-1] == ']') {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s\\]", elt->contents.str);
        } else {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s\\)", elt->contents.str);
        }
        break;
    default: 
        fprintf(stderr, "print_latex_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

/**********************************************************************

  Functions for printing Elements as groff (mm macros)

 ***********************************************************************/

static bool in_list_item = false; /* True if we're parsing contents of a list item. */

/* print_groff_string - print string, escaping for groff */
static void print_groff_string(GString *out, char *str) {
    while (*str != '\0') {
        switch (*str) {
        case '\\':
            g_string_append_printf(out, "\\e");
            break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_groff_mm_element_list - print a list of elements as groff ms */
static void print_groff_mm_element_list(GString *out, element *list) {
    int count = 1;
    while (list != NULL) {
        print_groff_mm_element(out, list, count);
        list = list->next;
        count++;
    }
}

/* print_groff_mm_element - print an element as groff ms */
static void print_groff_mm_element(GString *out, element *elt, int count) {
    int lev;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        padded = 0;
        break;
    case LINEBREAK:
        pad(out, 1);
        g_string_append_printf(out, ".br\n");
        padded = 0;
        break;
    case STR:
        print_groff_string(out, elt->contents.str);
        padded = 0;
        break;
    case ELLIPSIS:
        g_string_append_printf(out, "...");
        break;
    case EMDASH:
        g_string_append_printf(out, "\\[em]");
        break;
    case ENDASH:
        g_string_append_printf(out, "\\[en]");
        break;
    case APOSTROPHE:
        g_string_append_printf(out, "'");
        break;
    case SINGLEQUOTED:
        g_string_append_printf(out, "`");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "'");
        break;
    case DOUBLEQUOTED:
        g_string_append_printf(out, "\\[lq]");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\[rq]");
        break;
    case CODE:
        g_string_append_printf(out, "\\fC");
        print_groff_string(out, elt->contents.str);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case HTML:
        /* don't print HTML */
        break;
    case LINK:
        print_groff_mm_element_list(out, elt->contents.link->label);
        g_string_append_printf(out, " (%s)", elt->contents.link->url);
        padded = 0;
        break;
    case IMAGE:
        g_string_append_printf(out, "[IMAGE: ");
        print_groff_mm_element_list(out, elt->contents.link->label);
        g_string_append_printf(out, "]");
        padded = 0;
        /* not supported */
        break;
    case EMPH:
        g_string_append_printf(out, "\\fI");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case STRONG:
        g_string_append_printf(out, "\\fB");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case LIST:
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        lev = elt->key - H1 + 1;
        pad(out, 1);
        g_string_append_printf(out, ".H %d \"", lev);
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\"");
        padded = 0;
        break;
    case PLAIN:
        pad(out, 1);
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case PARA:
        pad(out, 1);
        if (!in_list_item || count != 1)
            g_string_append_printf(out, ".P\n");
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case HRULE:
        pad(out, 1);
        g_string_append_printf(out, "\\l'\\n(.lu*8u/10u'");
        padded = 0;
        break;
    case HTMLBLOCK:
        /* don't print HTML block */
        break;
    case VERBATIM:
        pad(out, 1);
        g_string_append_printf(out, ".VERBON 2\n");
        print_groff_string(out, elt->contents.str);
        g_string_append_printf(out, ".VERBOFF");
        padded = 0;
        break;
    case BULLETLIST:
        pad(out, 1);
        g_string_append_printf(out, ".BL");
        padded = 0;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".LE 1");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 1);
        g_string_append_printf(out, ".AL");
        padded = 0;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".LE 1");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, ".LI\n");
        in_list_item = true;
        padded = 2;
        print_groff_mm_element_list(out, elt->children);
        in_list_item = false;
        break;
    case BLOCKQUOTE:
        pad(out, 1);
        g_string_append_printf(out, ".DS I\n");
        padded = 2;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".DE");
        padded = 0;
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            g_string_append_printf(out, "\\*F\n");
            g_string_append_printf(out, ".FS\n");
            padded = 2;
            print_groff_mm_element_list(out, elt->children);
            pad(out, 1);
            g_string_append_printf(out, ".FE\n");
            padded = 1; 
        }
        break;
    case REFERENCE:
        /* Nonprinting */
        break;
    default: 
        fprintf(stderr, "print_groff_mm_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

/**********************************************************************

  Parameterized function for printing an Element.

 ***********************************************************************/

void print_element_list(GString *out, element *elt, int format, int exts) {
    /* Initialize globals */
    endnotes = NULL;
    notenumber = 0;

    /* And MultiMarkdown globals */
    base_header_level = 1;
    language = ENGLISH;
    html_footer = FALSE;
    
    extensions = exts;
    padded = 2;  /* set padding to 2, so no extra blank lines at beginning */

    format = find_latex_mode(format, elt);
    switch (format) {
    case HTML_FORMAT:
        print_html_element_list(out, elt, false);
        if (endnotes != NULL) {
            pad(out, 2);
            print_html_endnotes(out);
        }
        if (html_footer == TRUE) print_html_footer(out, false);
        break;
    case LATEX_FORMAT:
        print_latex_element_list(out, elt);
        break;
    case MEMOIR_FORMAT:
        print_memoir_element_list(out, elt);
        break;
    case BEAMER_FORMAT:
        print_beamer_element_list(out, elt);
        break;
	case OPML_FORMAT:
		g_string_append_printf(out, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<opml version=\"1.0\">\n");
		g_string_append_printf(out, "<body>\n");
		print_opml_element_list(out, elt);
		g_string_append_printf(out, "</body>\n</opml>");
		break;
    case GROFF_MM_FORMAT:
        print_groff_mm_element_list(out, elt);
        break;
    default:
        fprintf(stderr, "print_element - unknown format = %d\n", format); 
        exit(EXIT_FAILURE);
    }
}


/**********************************************************************

  MultiMarkdown Routines - Used for generating "complete" documents

 ***********************************************************************/


void print_html_header(GString *out, element *elt, bool obfuscate) {
    g_string_append_printf(out,
"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n");

    print_html_element_list(out, elt->children, obfuscate);
    g_string_append_printf(out, "</head>\n<body>\n");    
}


void print_html_footer(GString *out, bool obfuscate) {
    g_string_append_printf(out, "\n</body>\n</html>");
}


void print_latex_header(GString *out, element *elt) {
    print_latex_element_list(out, elt->children);
}


void print_latex_footer(GString *out) {
    if (latex_footer != NULL) {
        pad(out,2);
        g_string_append_printf(out, "\\input{%s}\n\n\\end{document}", latex_footer);
    }
}


/* print_memoir_element_list - print an element as LaTeX for memoir class */
void print_memoir_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_memoir_element(out, list);
        list = list->next;
    }
}


/* print_memoir_element - print an element as LaTeX for memoir class */
static void print_memoir_element(GString *out, element *elt) {
    int lev;
    char *label;
    switch (elt->key) {
    case VERBATIM:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{verbatim}\n\n");
        print_raw_element(out, elt);
        g_string_append_printf(out, "\n\\end{verbatim}\n\\end{adjustwidth}");
        padded = 0;
        break;
    case HEADINGSECTION:
        print_memoir_element_list(out, elt->children);
        break;
    case DEFLIST:
        g_string_append_printf(out, "\\begin{description}");
        padded = 0;
        print_memoir_element_list(out, elt->children);
        pad(out,1);
        g_string_append_printf(out, "\\end{description}");
        padded = 0;
        break;
    case DEFINITION:
        pad(out,2);
        padded = 2;
        print_memoir_element_list(out, elt->children);
        padded = 0;
        break;
    default:
        /* most things are not changed for memoir output */
        print_latex_element(out, elt);
    }
}


/* print_beamer_element_list - print an element as LaTeX for beamer class */
void print_beamer_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_beamer_element(out, list);
        list = list->next;
    }
}

static void print_beamer_endnotes(GString *out) {
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    pad(out,2);
    g_string_append_printf(out, "\\part{Bibliography}\n\\begin{frame}[allowframebreaks]\n\\frametitle{Bibliography}\n\\def\\newblock{}\n\\begin{thebibliography}{0}\n");
    while (note != NULL) {
        note_elt = note->data;
        pad(out, 1);
        g_string_append_printf(out, "\\bibitem{%s}\n", note_elt->contents.str);
        padded=2;
        print_latex_element_list(out, note_elt);
        pad(out, 1);
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "\\end{thebibliography}\n\\end{frame}\n\n");
    padded = 2;
    g_slist_free(endnotes);
}

/* print_beamer_element - print an element as LaTeX for beamer class */
static void print_beamer_element(GString *out, element *elt) {
    int lev;
    char *label;
    switch (elt->key) {
        case FOOTER:
            print_beamer_endnotes(out);
            g_string_append_printf(out, "\\mode<all>\n");
            print_latex_footer(out);
            g_string_append_printf(out, "\\mode*\n");
            break;
        case LISTITEM:
            pad(out, 1);
            g_string_append_printf(out, "\\item<+-> ");
            padded = 2;
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "\n");
            break;
        case HEADINGSECTION:
            if (elt->children->key -H1 + base_header_level == 3) {
                pad(out,2);
               g_string_append_printf(out, "\\begin{frame}");
                if (list_contains_key(elt->children,VERBATIM)) {
                    g_string_append_printf(out, "[fragile]");
                }
                padded = 0;
                print_beamer_element_list(out, elt->children);
                g_string_append_printf(out, "\n\n\\end{frame}\n\n");
                padded = 2;
            } else if (elt->children->key -H1 + base_header_level == 4) {
                pad(out, 1);
                g_string_append_printf(out, "\\mode<article>{\n");
                padded = 0;
                print_beamer_element_list(out, elt->children->next);
                g_string_append_printf(out, "\n\n}\n\n");
                padded = 2;
            } else {
                print_beamer_element_list(out, elt->children);
            }
            break;
        case H1: case H2: case H3: case H4: case H5: case H6:
            pad(out, 2);
            lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
            switch (lev) {
                case 1:
                    g_string_append_printf(out, "\\part{");
                    break;
                case 2:
                    g_string_append_printf(out, "\\section{");
                    break;
                case 3:
                    g_string_append_printf(out, "\\frametitle{");
                    break;
                default:
                    g_string_append_printf(out, "\\emph{");
                    break;
            }
            /* generate a label for each header (MMD)*/
            if (elt->children->key == AUTOLABEL) {
                label = label_from_string(elt->children->contents.str,0);
                print_latex_element_list(out, elt->children->next);
                g_string_append_printf(out, "}\n\\label{");
                g_string_append_printf(out, "%s", label);
                free(label);
            } else {
                label = label_from_element_list(elt->children,0);
                print_latex_element_list(out, elt->children);
                g_string_append_printf(out, "}\n\\label{");
                g_string_append_printf(out, "%s", label);
                free(label);
            }
            g_string_append_printf(out, "}\n");
            padded = 1;
            break;
        default:
        print_latex_element(out, elt);
    }
}


element * print_html_headingsection(GString *out, element *list, bool obfuscate) {
    element *base = list;
    print_html_element_list(out, list->children, obfuscate);
    
    list = list->next;
    while ( (list != NULL) && (list->key == HEADINGSECTION) && (list->children->key > base->children->key) && (list->children->key <= H6)) {
        list = print_html_headingsection(out, list, obfuscate);
    }

    return list;
}

bool list_contains_key(element *list, int key) {
    element *step = NULL;

    step = list;
    while ( step != NULL ) {
        if (step->key == key) {
            return TRUE;
        }
        if (step->children != NULL) {
            if (list_contains_key(step->children, key)) {
                return TRUE;
            }
        }
       step = step->next;
    }
    return FALSE;
}


/* look for "LaTeX Mode" metadata and change format to match */
static int find_latex_mode(int format, element *list) {
    element *latex_mode;
    char *label;
    
    if (format != LATEX_FORMAT) return format;
    
    if (list_contains_key(list,METAKEY)) {
        latex_mode = metadata_for_key("latexmode", list);
        if ( latex_mode != NULL) {
            label = label_from_element_list(latex_mode->children, 0);
            if (strcmp(label, "beamer") == 0) { format = BEAMER_FORMAT; } else 
            if (strcmp(label, "memoir") == 0) { format = MEMOIR_FORMAT; } 
            free(label);
        }
        return format;
    } else {
        return format;
    }
}


/* find specified metadata key, if present */
element * metadata_for_key(char *key, element *list) {
    element *step = NULL;
    step = list;
    char *label;
    
    label = label_from_string(key,0);
    
    while (step != NULL) {
        if (step->key == METADATA) {
           /* search METAKEY children */
            step = step->children;
            while ( step != NULL) {
                if (strcmp(step->contents.str, label) == 0) {
                    free(label);
                    return step;
                }
                step = step->next;
            }
            free(label);
            return NULL;
        }
       step = step->next;
    }
    free(label);
    return NULL;
}


/* find specified metadata key, if present */
char * metavalue_for_key(char *key, element *list) {
    element *step = NULL;
    step = list;
    char *label;
    char *result;
    
    label = label_from_string(key,0);
    
    while (step != NULL) {
        if (step->key == METADATA) {
           /* search METAKEY children */
            step = step->children;
            while ( step != NULL) {
                if (strcmp(step->contents.str, label) == 0) {
                    /* Found a match */
                    if ((strcmp(label,"latexmode") == 0) ||
                        (strcmp(label,"quoteslanguage") == 0)) {
                        result = label_from_string(step->children->contents.str,0);
                    } else {
                        result = strdup(step->children->contents.str);
                    }
                    free(label);
                   return result;
                }
                step = step->next;
            }
            free(label);
            return NULL;
        }
       step = step->next;
    }
    free(label);
    return NULL;
}

/* find attribute, if present */
element * element_for_attribute(char *querystring, element *list) {
    element *step = NULL;
    step = list;
    char *query;
    query = label_from_string(querystring,0);
    
    while (step != NULL) {
        if (strcmp(step->contents.str,query) == 0) {
            free(query);
            return step;
        }
        step = step->next;
    }
    free(query);
    return NULL;
}

/* convert attribute to dimensions suitable for LaTeX */
/* returns c string that needs to be freed */

char * dimension_for_attribute(char *querystring, element *list) {
    element *attribute;
    char *dimension;
    char *ptr;
    int i;
    char *upper;
    GString *result;

    attribute = element_for_attribute(querystring, list);
    if (attribute == NULL) return NULL;

    dimension = strdup(attribute->children->contents.str);
    upper = strdup(attribute->children->contents.str);

    for(i = 0; dimension[ i ]; i++)
        dimension[i] = tolower(dimension[ i ]);

    for(i = 0; upper[ i ]; i++)
        upper[i] = toupper(upper[ i ]);

    if (strstr(dimension, "px")) {
        ptr = strstr(dimension,"px");
        ptr[0] = '\0';
        strcat(ptr,"pt");
    }

    result = g_string_new(dimension);
    
    if (strcmp(dimension,upper) == 0) {
        /* no units */
        g_string_append_printf(result, "pt");
    }

    free(upper);
    free(dimension);
    
    dimension = result->str;
    g_string_free(result, false);
    return(dimension);
}

/* Check metadata keys and determine if I need a complete document */
static bool is_html_complete_doc(element *meta) {
    element *step;
    step = meta->children;
    
    while (step != NULL) {
        if (strcmp(step->contents.str, "baseheaderlevel") != 0) {
            if (strcmp(step->contents.str, "quoteslanguage") !=0 ){
                return TRUE;
            }
        }
        step = step->next;
    }
    
    return FALSE;
}


/* print_opml_element_list - print an element list as OPML */
void print_opml_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_opml_element(out, list);
        list = list->next;
    }
}

/* print_opml_element - print an element as OPML */
static void print_opml_element(GString *out, element *elt) {
    switch (elt->key) {
        case HEADINGSECTION:
			/* Need to handle "nesting" properly */
			g_string_append_printf(out, "<outline ");
			
			/* Print header */
			print_opml_element(out,elt->children);
			
			/* print remainder of paragraphs as note */
			g_string_append_printf(out, "_note=\"");
			print_opml_element_list(out,elt->children->next);
			g_string_append_printf(out, "\">");

			g_string_append_printf(out, "</outline>\n");
			break;
		case H1: case H2: case H3: case H4: case H5: case H6: 
			g_string_append_printf(out, "text=\"%s\" ", elt->contents.str);
			break;
	    case SPACE:
			print_opml_string(out, elt->contents.str);
	        break;
	    case STR:
	        print_opml_string(out, elt->contents.str);
	        break;
	    case LINEBREAK:
	        g_string_append_printf(out, "  &#10;");
	        break;
		case PLAIN:
			print_opml_element_list(out,elt->children);
			if ((elt->next != NULL) && (elt->next->key == PLAIN)) {
				g_string_append_printf(out, "&#10;&#10;");
			}
			break;
	    default: 
	        fprintf(stderr, "print_opml_element encountered unknown element key = %d\n", elt->key);
	        /*exit(EXIT_FAILURE);*/
    }
}

/* print_opml_string - print string, escaping for OPML */
static void print_opml_string(GString *out, char *str) {
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
		case '\n': case '\r':
			g_string_append_printf(out, "&#10;");
			break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}