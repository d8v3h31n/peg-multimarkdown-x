//
//  GLibCocoaBridge.h
//  MultiMarkdown
//
//  Created by Daniel Jalkut on 7/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef int gboolean;
typedef char gchar;

// WE implement minimal cover structs and associated call-through functions to implement
// glib's GString and GSList as NSMutableString and NSMutableArray, respectively.

// GString declarations

typedef struct 
{
	NSMutableString* cocoaString;
	
	// While we are buildign the string, we may receive UTF8 fragment bytes that can't be processed on their own,
	// so we queue them up.
	char utf8Fragments[5];
	
	// Compatibility entries for GString clients
	// Updated whenever the GString is manipulated
	char* str;
} GString;

GString* g_string_new(char *startingString);
char* g_string_free(GString* ripString, bool freeCharacterData);

void g_string_append_c(GString* baseString, char appendedCharacter);
void g_string_append(GString* baseString, char *appendedString);

void g_string_prepend(GString* baseString, char* prependedString);

void g_string_append_printf(GString* baseString, char* format, ...);

// GSList declarations

// Don't know yet how to tackle the problem of GSList bridging. It's a little tougher than
// GString because it strongly embraces a standard linked list paradigm. It might make most
// sense to just implement the simple linked list functionality, unless there's some CoreFoundation
// or POSIX library that is omni-present on Macs to support this.

typedef struct _GSList
{
	// And caches a reference to the data itself
	void* data;	
	struct _GSList* next;
} GSList;

void g_slist_free(GSList* ripList);
GSList* g_slist_prepend(GSList* targetElement, void* newElementData);
GSList* g_slist_reverse(GSList* theList);
