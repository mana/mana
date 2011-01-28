//
//  window.m
//  themanaworld
//
//  Created by David Athay on 1/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "window.h"
#import <Cocoa/Cocoa.h>

void setIcon(const char *icon)
{
    NSString *path = [NSString stringWithCString:icon
                                        encoding:[NSString defaultCStringEncoding]];
    NSString *imageName = [[NSBundle mainBundle] pathForResource:path ofType:@"icns" inDirectory:@"data/icons"];
    NSImage *iconImage = [[NSImage alloc] initWithContentsOfFile:imageName];
    [NSApp setApplicationIconImage: iconImage];
}

void windowAlert(const char *msg)
{
    NSString *alertText = [NSString stringWithCString:msg
                                             encoding:[NSString defaultCStringEncoding]];
    NSRunAlertPanel(nil, alertText , @"OK", nil, nil);
}