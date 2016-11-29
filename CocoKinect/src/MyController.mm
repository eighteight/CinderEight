//
//  MyController.mm
//  CCGLBlobDetection example
//
//  Created by Matthieu Savary on 03/03/11.
//  Copyright (c) 2011 SMALLAB.ORG. All rights reserved.
//
//  More info on the CCGL project >> http://www.smallab.org/code/ccgl/
//  License & disclaimer >> see license.txt file included in the distribution package
//

#import "MyController.h"


@implementation MyController


- (void)awakeFromNib {
    [NSApp setDelegate:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (IBAction)listenToHostText:(NSTextFieldCell *)sender {
    NSString *value = [sender stringValue];
    
    [CinderDrawing setOscAddress:value];
}
- (IBAction) listenToNumUsersButton:(NSSlider *)sender
{
	int value = [sender intValue];
	[CinderDrawing setNumUsers:value];
}


- (IBAction) listenToResetCameraButton: (NSButton*) sender
{
	[CinderDrawing resetCamera];
}

- (IBAction) listenToSendOSCButton: (NSButton*) sender
{
	[CinderDrawing toggleSendOsc];

}

- (IBAction) listenToShowInputButton: (NSButton*) sender
{
    [CinderDrawing toggleDisplayInput];
}

@end
