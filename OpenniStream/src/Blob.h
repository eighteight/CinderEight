//
//  Blob.h
//  AsioSerializationServer
//
//  Created by eight on 5/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "cinder/Vector.h"

#ifndef AsioSerializationServer_Blob_h
#define AsioSerializationServer_Blob_h

class Blob{
public:
    int id;
    float x;
    float y;
    float radius;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & x;
        ar & y;
        ar & radius;
    }

    
};


#endif
