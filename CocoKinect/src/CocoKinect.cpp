//
//  CocoKinect.cpp
//  CocoKinect
//
//  Created by Vladimir Gusev on 6/1/12.
//  Copyright onewaytheater.us. All rights reserved.
//

#include "CocoKinect.h"

#include "VOpenNICommon.h"
#include "cinder/Text.h"

const float ramScale = .1;
const static int RAM_JOINT_COUNT = 23;
namespace coco {
    
    
    enum Joint {
        
        // start at root joint
        JOINT_TORSO = 0,
        JOINT_NECK,
        JOINT_HEAD,
        
        // left arm + shoulder
        JOINT_LEFT_SHOULDER,
        JOINT_LEFT_ELBOW,
        JOINT_LEFT_HAND,
        
        // right arm + shoulder
        JOINT_RIGHT_SHOULDER,
        JOINT_RIGHT_ELBOW,
        JOINT_RIGHT_HAND,
        
        // left leg
        JOINT_LEFT_HIP,
        JOINT_LEFT_KNEE,
        JOINT_LEFT_FOOT,
        
        // right leg
        JOINT_RIGHT_HIP,
        JOINT_RIGHT_KNEE,
        JOINT_RIGHT_FOOT,
        
        JOINT_COUNT, //15
        JOINT_UNKOWN
    };
    
    
    int jointMapping[] = {
        JOINT_TORSO, //JOINT_HIPS
        JOINT_TORSO, //JOINT_ABDOMEN,
        JOINT_NECK, //JOINT_CHEST,
        JOINT_NECK, //JOINT_NECK,
        JOINT_HEAD, //JOINT_HEAD,
        JOINT_LEFT_HIP, //JOINT_LEFT_HIP,
        JOINT_LEFT_KNEE, //JOINT_LEFT_KNEE,
        JOINT_LEFT_FOOT, //JOINT_LEFT_ANKLE,
        JOINT_LEFT_FOOT, //JOINT_LEFT_TOE,
        JOINT_RIGHT_HIP, //JOINT_RIGHT_HIP,
        JOINT_RIGHT_KNEE, //JOINT_RIGHT_KNEE,
        JOINT_RIGHT_FOOT, //JOINT_RIGHT_ANKLE,
        JOINT_RIGHT_FOOT, //JOINT_RIGHT_TOE,
        JOINT_LEFT_SHOULDER, //JOINT_LEFT_COLLAR,
        JOINT_LEFT_SHOULDER, //JOINT_LEFT_SHOULDER,
        JOINT_LEFT_ELBOW, //JOINT_LEFT_ELBOW,
        JOINT_LEFT_HAND, //JOINT_LEFT_WRIST,
        JOINT_LEFT_HAND, //JOINT_LEFT_HAND,
        JOINT_RIGHT_SHOULDER, //JOINT_RIGHT_COLLAR,
        JOINT_RIGHT_SHOULDER, //JOINT_RIGHT_SHOULDER,
        JOINT_RIGHT_ELBOW, //JOINT_RIGHT_ELBOW,
        JOINT_RIGHT_HAND, //JOINT_RIGHT_WRIST,
        JOINT_RIGHT_HAND, //JOINT_RIGHT_HAND,
    };

    int jointMappingNew[] = {    
    V::SKEL_TORSO,
    V::SKEL_TORSO,
    V::SKEL_NECK,
    V::SKEL_NECK,
    V::SKEL_HEAD,
    V::SKEL_LEFT_HIP,
    V::SKEL_LEFT_KNEE,
    V::SKEL_LEFT_ANKLE,
    V::SKEL_LEFT_FOOT,
    V::SKEL_RIGHT_HIP,
    V::SKEL_RIGHT_KNEE,
    V::SKEL_RIGHT_FOOT,
    V::SKEL_RIGHT_FOOT,
    V::SKEL_LEFT_SHOULDER,
    V::SKEL_LEFT_SHOULDER,
    V::SKEL_LEFT_ELBOW,
    V::SKEL_LEFT_HAND,
    V::SKEL_LEFT_HAND,
    V::SKEL_RIGHT_SHOULDER,
    V::SKEL_RIGHT_SHOULDER,
    V::SKEL_RIGHT_ELBOW,
    V::SKEL_RIGHT_HAND,
    V::SKEL_RIGHT_HAND
    };

    
    string ramJointName[] =
    {
        "HIPS",
        "ABDOMEN",
        "CHEST",
        "NECK",
        "HEAD",
        "LEFT_HIP",
        "LEFT_KNEE",
        "LEFT_ANKLE",
        "LEFT_TOE",
        "RIGHT_HIP",
        "RIGHT_KNEE",
        "RIGHT_ANKLE",
        "RIGHT_TOE",
        "LEFT_COLLAR",
        "LEFT_SHOULDER",
        "LEFT_ELBOW",
        "LEFT_WRIST",
        "LEFT_HAND",
        "RIGHT_COLLAR",
        "RIGHT_SHOULDER",
        "RIGHT_ELBOW",
        "RIGHT_WRIST",
        "RIGHT_HAND"
    };
}
CocoKinect::CocoKinect()
{
	pixels = NULL;
}
CocoKinect::~CocoKinect()
{
	delete [] pixels;
	pixels = NULL;
}

void CocoKinect::fileDrop( FileDropEvent event )
{
        if (_device0.get()){
            _device0->release();
            _device0.reset();
        }
        _manager->Release();
        
        _manager = V::OpenNIDeviceManager::InstancePtr();
    std::string fileName =  event.getFile(0).c_str();
    isDevice = _manager->createDevice(event.getFile(0).c_str(), V::NODE_TYPE_DEPTH | V::NODE_TYPE_USER | V::NODE_TYPE_SCENE, 0 );

    if (isDevice && setDevice()){
         status = "Playing file " +fileName;
    } else {
        status = "Plug in camera and Reset or drop *.oni file here.";
    }

}

void CocoKinect::reset(){
    if (_device0.get()){
        _device0->release();
        _device0.reset();
    }
    _manager->Release();

    _manager = V::OpenNIDeviceManager::InstancePtr();
    isDevice = _manager->createDevices( 1, V::NODE_TYPE_DEPTH | V::NODE_TYPE_SCENE | V::NODE_TYPE_USER );

    if (setDevice()){
        status = "Kinect connected";
    } else {
        status = "Plug in camera and Reset or drop *.oni file here.";
    }
}

bool CocoKinect::setDevice(){
    _device0 = _manager->getDevice(0);
    
	if( _device0 )
	{
        _device0->setDepthShiftMul( 3 );
        
        _device0->addListener( this );
        _manager->start();
        _manager->setMaxNumOfUsers(numUsers);
        
        _device0->setDepthShiftMul( 3 );
        return true;
	} else{
        return false;
    }
}

void CocoKinect::prepareSettings( Settings *settings )
{
	settings->setFrameRate( 30 );
	settings->setWindowSize( WIDTH, HEIGHT );
}

void CocoKinect::setup()
{
    openniCenter = Vec3f(0, 0, 2000);
    mFont = Font( "Times New Roman", 12.0f );
    
    V::OpenNIDeviceManager::USE_THREAD = false;
    _manager = V::OpenNIDeviceManager::InstancePtr();
    isDevice = _manager->createDevices( 1, V::NODE_TYPE_DEPTH | V::NODE_TYPE_SCENE | V::NODE_TYPE_USER );
    numUsers = 1;
    if (setDevice()){
        status = "Kinect connected";
    } else {
        status = "Plug in camera and Reset or drop *.oni file here.";
    }

	pixels = new uint16_t[ KINECT_DEPTH_SIZE ];

    host = "localhost";
	port = 10000;
    
    sender.setup( host, port, true );
    isDisplayInput = true;
    isSendOsc = true;
}

void CocoKinect::toggleDisplayInput(){
    isDisplayInput = !isDisplayInput;
}

void CocoKinect::toggleSendOsc(){
    isSendOsc = !isSendOsc;
}


void CocoKinect::mouseDown( MouseEvent event )
{
}

void CocoKinect::setNumUsers(uint32_t nUsers){
    if (!_device0) return;
    _manager->setMaxNumOfUsers(nUsers);
    numUsers = nUsers;
}

void CocoKinect::setOscAddress(std::string addr){
    host = addr;
    sender.setup(host, 10000);
}

void CocoKinect::update()
{
    try {
        if (!_device0 || !isDevice) return;
        
        if( !V::OpenNIDeviceManager::USE_THREAD ){
            _manager->update();
        }
        
        // Update textures
        //mColorTex = getColorImage();
        mDepthTex = getDepthImage();
        
        // Uses manager to handle users.
        for( std::map<int, gl::Texture>::iterator it=mUsersTexMap.begin(); it != mUsersTexMap.end(); ++it ){
            it->second = getUserImage( it->first );
        }
        
        if( _manager->hasUsers()){        
            _manager->updatePositions(userPositions);
            if (isSendOsc){
                for (boost::unordered_map<int, vector<Vec3f> >::iterator it = userPositions.begin(); it != userPositions.end(); ++it){
                    serializeSkeleton((*it).first, (*it).second);
                }
            }
	}
    } catch (...){
        status = "Cannot update device";
    }
}

void CocoKinect::draw(){    
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
	gl::setMatricesWindow( WIDTH, HEIGHT );
    float sy = 180.0f;
    float sx = sy*1.5f;
    
    int xoff = 2;
    int yoff = 34;
    if (isDisplayInput){
        gl::disableDepthWrite();
        gl::disableDepthRead();
        

        //glEnable( GL_TEXTURE_2D );
        gl::color( cinder::ColorA(1, 1, 1, 1) );
        if (mColorTex)
            gl::draw( mColorTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
        if (mDepthTex){
            if (mColorTex)
                gl::draw( mDepthTex, Rectf( xoff*2+sx*1, yoff, xoff+sx*2, yoff+sy) );
            else
                gl::draw( mDepthTex,Rectf( 0, 0, WIDTH, HEIGHT) );
        }
    }

    gl::drawString(status, Vec2f(5.0f, ((float)HEIGHT) - 16.0f), ColorA( 1, 1, 1, 1 ), mFont);
    
    if(isDisplayInput && _manager->hasUsers()) {
        gl::disable( GL_TEXTURE_2D );
        //gl::translate(cinder::Vec2f(xoff*2+sx*1,yoff));
        _manager->renderJoints( WIDTH, HEIGHT, 1, 3, false );
    }
}

using namespace coco;
void CocoKinect::serializeSkeleton(int id, vector<Vec3f> positions){
    
    osc::Bundle bundle;
    osc::Message msg;
    
    msg.setAddress("/ram/skeleton");
    std::ostringstream actorName;
    actorName <<"Actor "<<id;
    msg.addStringArg(actorName.str());
    msg.addIntArg(RAM_JOINT_COUNT);
    
    float floorOffset = 0;
    for(int openniIndex = 0; openniIndex < positions.size(); openniIndex++) {
        positions[openniIndex].x *= -1; // openni is mirrored left/right
        if(openniIndex == 0 || positions[openniIndex].y < floorOffset) {
            floorOffset = positions[openniIndex].y;
        }
    }
    
    Vec3f torsoToNeck = positions[V::SKEL_NECK] - positions[V::SKEL_TORSO];
    Vec3f torsoToLeftHip = positions[V::SKEL_LEFT_HIP] - positions[V::SKEL_TORSO];
    Vec3f torsoToRightHip = positions[V::SKEL_RIGHT_HIP] - positions[V::SKEL_TORSO];
    Vec3f neckToTorso = positions[V::SKEL_TORSO] - positions[V::SKEL_NECK];
    Vec3f neckToRightShoulder = positions[V::SKEL_RIGHT_SHOULDER] - positions[V::SKEL_NECK];
    Vec3f headToNeck = positions[V::SKEL_NECK] - positions[V::SKEL_HEAD];
    Vec3f leftShoulderToLeftElbow = positions[V::SKEL_LEFT_ELBOW] - positions[V::SKEL_LEFT_SHOULDER];
    Vec3f leftElbowToLeftHand = positions[V::SKEL_LEFT_HAND] - positions[V::SKEL_LEFT_ELBOW];
    Vec3f rightShoulderToRightElbow = positions[V::SKEL_RIGHT_ELBOW] - positions[V::SKEL_RIGHT_SHOULDER];
    Vec3f rightElbowToRightHand = positions[V::SKEL_RIGHT_HAND] - positions[V::SKEL_RIGHT_ELBOW];
    Vec3f leftHipToLeftKnee = positions[V::SKEL_LEFT_KNEE] - positions[V::SKEL_LEFT_HIP];
    Vec3f leftKneeToLeftFoot = positions[V::SKEL_LEFT_FOOT] - positions[V::SKEL_LEFT_KNEE];
    Vec3f rightHipToRightKnee = positions[V::SKEL_RIGHT_KNEE] - positions[V::SKEL_RIGHT_HIP];
    Vec3f rightKneeToRightFoot = positions[V::SKEL_RIGHT_FOOT] - positions[V::SKEL_RIGHT_KNEE];
    
    vector<Quaternion<float> > orientations = vector<Quaternion<float> >(JOINT_COUNT, Quaternion<float>());

    orientations[JOINT_TORSO] = Quaternion<float>(torsoToNeck, torsoToRightHip);
    orientations[JOINT_NECK] = Quaternion<float>(neckToTorso, neckToRightShoulder);
    orientations[JOINT_HEAD] = Quaternion<float>(headToNeck, neckToRightShoulder);
    orientations[JOINT_LEFT_SHOULDER] = Quaternion<float>(leftShoulderToLeftElbow, neckToRightShoulder);
    orientations[JOINT_LEFT_ELBOW] = Quaternion<float>(leftElbowToLeftHand, leftShoulderToLeftElbow);
    orientations[JOINT_LEFT_HAND] = orientations[JOINT_LEFT_ELBOW];
    orientations[JOINT_RIGHT_SHOULDER] = Quaternion<float>(rightShoulderToRightElbow, neckToRightShoulder);
    orientations[JOINT_RIGHT_ELBOW] = Quaternion<float>(rightElbowToRightHand, rightShoulderToRightElbow);
    orientations[JOINT_RIGHT_HAND] = orientations[JOINT_RIGHT_ELBOW];
    orientations[JOINT_LEFT_HIP] = Quaternion<float>(leftHipToLeftKnee, torsoToLeftHip);
    orientations[JOINT_LEFT_KNEE] = Quaternion<float>(leftKneeToLeftFoot, leftHipToLeftKnee);
    orientations[JOINT_LEFT_FOOT] = orientations[JOINT_LEFT_KNEE];
    orientations[JOINT_RIGHT_HIP] = Quaternion<float>(rightHipToRightKnee, torsoToRightHip);
    orientations[JOINT_RIGHT_KNEE] = Quaternion<float>(rightKneeToRightFoot, rightHipToRightKnee);
    orientations[JOINT_RIGHT_FOOT] = orientations[JOINT_RIGHT_KNEE];
    
    for(int ramIndex = 0; ramIndex < RAM_JOINT_COUNT; ramIndex++) {
        int openniIndex = jointMappingNew[ramIndex]-1;
        Vec3f position = positions[openniIndex];
        if (ramIndex == 0) position = (positions[V::SKEL_RIGHT_HIP-1]+positions[V::SKEL_LEFT_HIP-1])*0.5;
        position -= openniCenter;
        position.y -= floorOffset;
        position *= ramScale;
        string nam = ramJointName[ramIndex];
        msg.addStringArg(nam);
        //std::cout<<ramJointNameNew[ramIndex]<<" "<<ramIndex<<std::endl;
        msg.addFloatArg(position.x);
        msg.addFloatArg(position.y);
        msg.addFloatArg(position.z);
        // send zero orientation
        Quaternion<float>& orientation = orientations[openniIndex];
        Vec3f axis = orientation.getAxis();
        msg.addFloatArg(orientation.getAngle());
        msg.addFloatArg(axis.x);
        msg.addFloatArg(axis.y);
        msg.addFloatArg(axis.z);
    }
    msg.addFloatArg(getElapsedSeconds());
    
    bundle.addMessage(msg);
    sender.sendBundle(bundle);
}

void CocoKinect::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
		this->quit();
		this->shutdown();
	}
}

void CocoKinect::shutdown(){
    
}


void CocoKinect::onNewUser( V::UserEvent event )
{
    std::ostringstream frmrtStrm;
	app::console() << "New User Added With ID: " << event.mId << std::endl;

    mUsersTexMap.insert( std::make_pair( event.mId, gl::Texture(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT) ) );
}


void CocoKinect::onLostUser( V::UserEvent event )
{
	app::console() << "User Lost With ID: " << event.mId << std::endl;
    mUsersTexMap.erase( event.mId );
}

