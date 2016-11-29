//
//  MyCinderGLDrawing.h
//  CCGLBlobDetection example
//
//  Created by Matthieu Savary on 03/03/11.
//  Copyright (c) 2011 SMALLAB.ORG. All rights reserved.
//
//  More info on the CCGL project >> http://www.smallab.org/code/ccgl/
//  License & disclaimer >> see license.txt file included in the distribution package
//


#include "CCGLView.h"

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/Rand.h"
#include "CocoKinect.h"


@interface MyCinderGLDrawing : CCGLView
{
	gl::Texture		mTexture;
    int             mWidth, mHeight;

    int             nCutoff, fCutoff;
    boolean_t       mDrawCapture;
    
    CocoKinect cocoKinect;
}

/**
 *  Cocoa UI methods
 */

- (void) setNumUsers: (int) numUsers;
- (void) resetCamera;
- (void) toggleSendOsc;
- (void) toggleDisplayInput;
- (void) dragAndDropFiles:(NSArray *)filenames;
- (void) setOscAddress:NSString;

@end
