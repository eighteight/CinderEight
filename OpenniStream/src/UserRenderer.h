//
//  SkeletonRenderer.h
//  BlockOpenNISkeleton
//
//  Created by Vladimir Gusev on 5/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef BlockOpenNISkeleton_SkeletonRenderer_h
#define BlockOpenNISkeleton_SkeletonRenderer_h

// Enumeration for skeleton joints
enum SkeletonBoneType
{
    SKEL_HEAD = 1, 
    SKEL_NECK = 2, 
    SKEL_TORSO = 3, 
    SKEL_WAIST = 4, 
    
    SKEL_LEFT_COLLAR = 5, 
    SKEL_LEFT_SHOULDER = 6, 
    SKEL_LEFT_ELBOW = 7, 
    SKEL_LEFT_WRIST = 8, 
    SKEL_LEFT_HAND = 9, 
    SKEL_LEFT_FINGERTIP = 10, 
    
    SKEL_RIGHT_COLLAR = 11, 
    SKEL_RIGHT_SHOULDER = 12, 
    SKEL_RIGHT_ELBOW = 13, 
    SKEL_RIGHT_WRIST = 14, 
    SKEL_RIGHT_HAND = 15, 
    SKEL_RIGHT_FINGERTIP = 16, 
    
    SKEL_LEFT_HIP = 17, 
    SKEL_LEFT_KNEE = 18, 
    SKEL_LEFT_ANKLE = 19, 
    SKEL_LEFT_FOOT = 20, 
    
    SKEL_RIGHT_HIP = 21, 
    SKEL_RIGHT_KNEE = 22, 
    SKEL_RIGHT_ANKLE = 23, 
    SKEL_RIGHT_FOOT = 24,
    SKEL_TOTAL_COUNT = 24
};

class Bone{
public:
    int id;
    cinder::Vec3f position;
    float positionConfidence, orientationConfidence;
    
};
class UserRenderer{
public:
    std::vector<Bone> mBoneList;
    UserRenderer(){};
    void UserRenderer::setSkeleton(std::vector<Bone> boneList){
        mBoneList = boneList;
    }
    ~UserRenderer(){
        
    }

    void UserRenderer::draw(float width, float height, float depth, float pointSize, bool renderDepth ){
        glBegin( GL_QUADS );

        for(std::vector<Bone>::size_type index = 0; index != mBoneList.size(); index++)
        {
            Bone bone = mBoneList[index];

            // Bail out in case any bone is not confident
            if( bone.positionConfidence < 0.5f || bone.orientationConfidence < 0.5f )
                break;
            
            cinder::Vec3f point;
            point.x = bone.position.x;
            point.y = bone.position.y;
            point.z = bone.position.z;

            point.x *= 0.0015625f;			// div by 640
            point.y *= 0.00208333333333333333333333333333f;			// div by 480
            float sx = pointSize;
            float sy = pointSize;
            if( bone.id == SKEL_TORSO )
            {
                sx *= 2;
                sy *= 2;
            }
            
            if( bone.id == SKEL_TORSO )
                glColor4f( 1, 0, 0, 1 );
            else
                glColor4f( 1, 1, 1, 1 );
            if( renderDepth )
            {
                glVertex3f( -sx+(point.x*width), -sy+(point.y*height), point.z*depth );
                glVertex3f(  sx+(point.x*width), -sy+(point.y*height), point.z*depth );
                glVertex3f(  sx+(point.x*width),  sy+(point.y*height), point.z*depth );
                glVertex3f( -sx+(point.x*width),  sy+(point.y*height), point.z*depth );
                
            }
            else
            {
                glVertex3f( -sx+(point.x*width), -sy+(point.y*height), 0 );
                glVertex3f(  sx+(point.x*width), -sy+(point.y*height), 0 );
                glVertex3f(  sx+(point.x*width),  sy+(point.y*height), 0 );
                glVertex3f( -sx+(point.x*width),  sy+(point.y*height), 0 );
            }
        }
        glEnd();
        
        
        //
        // Render body connecting lines
        //
//        renderBone( SKEL_HEAD, SKEL_NECK, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_LEFT_SHOULDER, width, height, depth, true, renderDepth );
//        renderBone( SKEL_LEFT_SHOULDER, SKEL_LEFT_ELBOW, width, height, depth, true, renderDepth );
//        renderBone( SKEL_LEFT_ELBOW, SKEL_LEFT_HAND, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_RIGHT_SHOULDER, width, height, depth, true, renderDepth );
//        renderBone( SKEL_RIGHT_SHOULDER, SKEL_RIGHT_ELBOW, width, height, depth, true, renderDepth );
//        renderBone( SKEL_RIGHT_ELBOW, SKEL_RIGHT_HAND, width, height, depth, true, renderDepth );
//        
//        //			renderBone( SKEL_LEFT_SHOULDER, SKEL_TORSO, width, height, depth, true, renderDepth );
//        //			renderBone( SKEL_RIGHT_SHOULDER, SKEL_TORSO, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_LEFT_HIP, width, height, depth, true, renderDepth );
//        renderBone( SKEL_LEFT_HIP, SKEL_LEFT_KNEE, width, height, depth, true, renderDepth );
//        renderBone( SKEL_LEFT_KNEE, SKEL_LEFT_FOOT, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_RIGHT_HIP, width, height, depth, true, renderDepth );
//        renderBone( SKEL_RIGHT_HIP, SKEL_RIGHT_KNEE, width, height, depth, true, renderDepth );
//        renderBone( SKEL_RIGHT_KNEE, SKEL_RIGHT_FOOT, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_RIGHT_HIP, width, height, depth, true, renderDepth );
//        renderBone( SKEL_TORSO, SKEL_LEFT_HIP, width, height, depth, true, renderDepth );
//        //			renderBone( SKEL_LEFT_HIP, SKEL_RIGHT_HIP, width, height, depth, true, renderDepth );
//        
//        renderBone( SKEL_TORSO, SKEL_NECK, width, height, depth, true, renderDepth );
        
        // Restore texture
        //glEnable( GL_TEXTURE_2D );

    }
};



#endif
