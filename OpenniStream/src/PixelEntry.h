//
//  PixelEntry.h
//  BlockOpenNISkeleton
//
//  Created by Vladimir Gusev on 5/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef BlockOpenNISkeleton_PixelEntry_h
#define BlockOpenNISkeleton_PixelEntry_h
template <class T>
class PixelEntry{
public:
    PixelEntry(){};
    PixelEntry(T* pixels, size_t size):pixels(pixels), size(size){
    }
    
    ~PixelEntry(){}

    T* pixels;
    size_t size;
};


#endif
