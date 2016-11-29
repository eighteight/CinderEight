//
//  MyController.h
//  CCGLBlobDetection example
//
//  Created by Matthieu Savary on 03/03/11.
//  Copyright (c) 2011 SMALLAB.ORG. All rights reserved.
//
//  More info on the CCGL project >> http://www.smallab.org/code/ccgl/
//  License & disclaimer >> see license.txt file included in the distribution package
//


@interface MyController : NSObject {
	IBOutlet id CinderDrawing;
}

- (IBAction) listenToOscAddresText: (NSTextField*) sender;
- (IBAction) listenToSendOSCButton: (NSButton*) sender;
- (IBAction) listenToShowInputButton: (NSButton*) sender;
- (IBAction) listenToNumUsersButton: (NSButton*) sender;
- (BOOL)    applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender;
- (IBAction)listenToHostText:(NSTextFieldCell *)sender;

@end