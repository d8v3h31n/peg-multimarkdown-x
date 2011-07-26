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
	
	// Compatibility entries for GString clients
	// Updated whenever the GString is manipulated
	char* str;
} GString;

GString* g_string_new(char *startingString);
char* g_string_free(GString* ripString, bool freeCharacterData);

void g_string_append_c(GString* baseString, char appendedCharacter);
void g_string_append(GString* baseString, char *appendedString);

void g_string_prepend(GString* baseString, char* prependedString);

#define g_string_append_printf(baseString, appendedFormat, ...)	g_string_append(baseString, (char*)[[NSString stringWithFormat:[NSString stringWithUTF8String:appendedFormat], ## __VA_ARGS__] UTF8String])

// GSList declarations

typedef struct _GSList
{
	// Data is all managed by a real array
	NSMutableArray* cocoaArray;
	
	// Each "linked list item" (GSList) knows its own object from the array
	id thisArrayItem;

	// And caches a reference to the data itself
	void* data;	
	struct _GSList* next;
} GSList;

GSList* g_slist_prepend(GSList* targetElement, void* newElementData);
