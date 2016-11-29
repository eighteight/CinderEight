//
//  MyCinderGLDrawing.mm
//  CCGLBlobDetection example
//
//  Created by Matthieu Savary on 03/03/11.
//  Copyright (c) 2011 SMALLAB.ORG. All rights reserved.
//
//  More info on the CCGL project >> http://www.smallab.org/code/ccgl/
//  License & disclaimer >> see license.txt file included in the distribution package
//

#import "MyCinderGLDrawing.h"

@implementation MyCinderGLDrawing

/**
 *  The superclass setup method
 */

- (void) setup
{
	[super setup];
	
	// set initial values
    mWidth = 640;
    mHeight = 480;
    mDrawCapture = true;
    
    cocoKinect.setup();
}

- (void) update
{
	[super update];
    
    cocoKinect.update();
}



/**
 *  The superclass draw loop method
 */

- (void) draw
{    
    cocoKinect.draw();
}




/**
 *  Cocoa UI methods
 */


- (void)resetCamera
{
    cocoKinect.reset();
}
- (void)toggleSendOsc
{
    cocoKinect.toggleSendOsc();
}

- (void) setOscAddress:(NSString *)value {
    std::string filepath = [value cStringUsingEncoding:[NSString defaultCStringEncoding]];
	
    cocoKinect.setOscAddress(filepath);
}
- (void) setNumUsers: (int) numUsers{
    cocoKinect.setNumUsers((uint32_t)numUsers);
}

- (void) dragAndDropFiles:(NSArray *)filenames;
{
    std::vector<cinder::fs::path> paths;
    int numberOfFiles = [filenames count];
    for( int i = 0; i < numberOfFiles; ++i )
        paths.push_back( cinder::fs::path( [[filenames objectAtIndex:i] UTF8String] ) );
    
    cinder::app::FileDropEvent fileDropEvent(nil, 0, 0, paths );
    cocoKinect.fileDrop( fileDropEvent);
}

- (void) toggleDisplayInput{
    cocoKinect.toggleDisplayInput();
}

/**
 *  Superclass events
 */

- (void)reshape
{
	[super reshape];
}

- (void)mouseDown:(NSEvent*)theEvent
{
	[super mouseDown:(NSEvent *)theEvent];

	//console() << "MouseDown x : " << curPoint.x << ", y : " << curPoint.y << endl;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[super mouseDragged:(NSEvent *)theEvent];

}

- (void)keyDown:(NSEvent*)theEvent
{
	[super keyDown:(NSEvent *)theEvent];
}

@end
