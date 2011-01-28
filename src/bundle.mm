//
//  bundle.m
//  themanaworld
//
//  Created by David Athay on 1/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "bundle.h"
#import <Cocoa/Cocoa.h>

std::string getBundleResourcesPath()
{
    std::string resPath;
    NSBundle *mainBundle;
    
    // Get the main bundle for the app.
    mainBundle = [NSBundle mainBundle];
    
    NSString *bundlePath = [mainBundle bundlePath];
    NSArray *bundlePathArray = [[NSFileManager defaultManager] directoryContentsAtPath:bundlePath];
    // check it contains the right directories
    if ((nil != bundlePathArray) && ([bundlePathArray containsObject:@"Contents"]))
    {
        NSString *contentsPath = [bundlePath stringByAppendingPathComponent:@"Contents"];
        NSArray *contentsPathArray = [[NSFileManager defaultManager] directoryContentsAtPath:contentsPath];
        if ((nil != contentsPath) && ([contentsPathArray containsObject:@"MacOS"]) && ([contentsPathArray containsObject:@"Resources"]))
        {
            // get the final path of the resources
            NSString *finalResourcesPath = [contentsPath stringByAppendingPathComponent:@"Resources"];
            resPath = [finalResourcesPath UTF8String];
        }
    }
    
    return resPath;
}