//
//  GLibCocoaBridge.m
//  MultiMarkdown
//
//  Created by Daniel Jalkut on 7/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GLibCocoaBridge.h"

// GString

void recacheUTF8String(GString* theGString)
{
	// We always keep a complete copy ... potentially expensive, yes, but it
	// is necessary in case the client has grabbed theGString->str just before
	// freeing us without freeing the str representation.
	if (theGString->str != NULL)
	{
		free(theGString->str);
	}
	char* utf8String = [theGString->cocoaString UTF8String];
	NSUInteger stringLength = strlen(utf8String);
	theGString->str = malloc(stringLength + 1);
	strncpy(theGString->str, utf8String, stringLength);
}

GString* g_string_new(char *startingString)
{
	GString* newString = malloc(sizeof(GString));

	newString->cocoaString = [[NSMutableString stringWithUTF8String:startingString] retain];
	newString->str = NULL;
	
	recacheUTF8String(newString);
	
	return newString;
}

char* g_string_free(GString* ripString, bool freeCharacterData)
{	
	char* returnedString = NULL;
	if (freeCharacterData)
	{
		if (ripString->str != NULL)
		{
			free(ripString->str);
		}
	}
	
	[ripString->cocoaString release];
	free(ripString);
	
	return returnedString;
}

void g_string_append_c(GString* baseString, char appendedCharacter)
{
	[baseString->cocoaString appendString:[NSString stringWithFormat:@"%c", appendedCharacter]];
	recacheUTF8String(baseString);
}

void g_string_append(GString* baseString, char* appendedString)
{
	[baseString->cocoaString appendString:[NSString stringWithUTF8String:appendedString]];
	recacheUTF8String(baseString);
}

void g_string_append_printf(GString* baseString, char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	// Thanks to Mike Ash for the tip about using initWithFormat:arguments: to work around warnings
	// when the caller doesn't provide any varargs.
	NSString* appendedString = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:format] arguments:args];
	g_string_append(baseString, (char*)[appendedString UTF8String]);
} 

void g_string_prepend(GString* baseString, char* prependedString)
{
	[baseString->cocoaString insertString:[NSString stringWithUTF8String:prependedString] atIndex:0];
	recacheUTF8String(baseString);
}

// GSList

void g_slist_free(GSList* ripList)
{
#warning not implemented
	NSLog(@"Can't free lists yet");
}

GSList* g_slist_reverse(GSList* theList)
{
#warning not implemented
	NSLog(@"Can't reverse lists yet");
}

GSList* g_slist_prepend(GSList* targetElement, void* newElementData)
{
	NSLog(@"Can't prepend lists yet");
}

