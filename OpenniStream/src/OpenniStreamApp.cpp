/***
	SkeletonApp

	A sample app showing skeleton rendering with the kinect and openni.
	This sample renders only the user with id=1. If user has another id he won't be displayed.
	You may change that in the code.

	V.
***/


#include "cinder/app/AppBasic.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "VOpenNIHeaders.h"
#include "OscSender.h"
#include "UserRenderer.h"
#include "ConcurrentQueue.h"
#include "CinderVideoStreamServer.h"
#include "CinderVideoStreamServerV.h"
#include "CinderOpenCv.h"
#include <zlib.h>
#include "Blob.h"
#include "PixelEntry.h"
#include "Util.h"
#include "KinectTextures.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class openniStreamApp : public AppBasic, V::UserListener
{
public:
	static const int WIDTH = 800;
	static const int HEIGHT = 600;

	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
    static const int KINECT_DEPTH_SIZE = KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH;
	static const int KINECT_DEPTH_FPS = 30;

    osc::Sender sender;
	std::string host;
	int port;
    std::string skelStr;
    boost::asio::io_service io_service;
    Surface16u originalDepthSurface;
    Surface16u resizedSurface;

	openniStreamApp();
	~openniStreamApp();
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void shutdown();
	void keyDown( KeyEvent event );
    void serializeUser();
    void serializeSkeleton();
    void serializeUserPixels();

	void onNewUser( V::UserEvent event );
	void onLostUser( V::UserEvent event );

	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}

	ImageSourceRef getUserImage( int id )
	{
		_device0->getLabelMap( id, pixels );
		return ImageSourceRef( new ImageSourceKinectDepth( pixels, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}

	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	} 

	void prepareSettings( Settings *settings );

private:
    void queueUser();
    // Members
    V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;

	gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
    std::map<int, gl::Texture> mUsersTexMap;
	
	uint16_t*				pixels;
    uint16_t *              previousPixels;
    uint16_t*               pixelDiff;

    uLong                   diffSize;
    Font mFont;
    
    double                   startTime, endTime, frameTime;
    UserRenderer skeletonRenderer;
    std::string::size_type lenSkel, lenUsrPx;
    std::string pixStr, pixStrZ, skelStrZ;

    ph::ConcurrentQueue<uint16_t*>* queueToServer;
    ph::ConcurrentQueue<PixelEntry<uint16_t> >* queueToServerV;
    void threadLoopV();
    bool running;
};

void openniStreamApp::threadLoopV()
{
    while (running) {
        try {
            boost::shared_ptr<CinderVideoStreamServerV<uint16_t> > server = boost::shared_ptr<CinderVideoStreamServerV<uint16_t> >(new CinderVideoStreamServerV<uint16_t>(3333, queueToServerV, KINECT_DEPTH_SIZE));
            server.get()->run();
        }
        catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
        //boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

openniStreamApp::openniStreamApp() 
{
	pixels = NULL;
    previousPixels = NULL;
    pixelDiff = NULL;
}
openniStreamApp::~openniStreamApp()
{
	delete [] pixels;
	pixels = NULL;
    delete [] previousPixels;
    previousPixels = NULL;
    
    delete [] pixelDiff;
    pixelDiff = NULL;
}

void openniStreamApp::prepareSettings( Settings *settings )
{
	settings->setFrameRate( 30 );
	settings->setWindowSize( WIDTH, HEIGHT );
	settings->setTitle( "BlockOpenNI Skeleton Sample" );
}

void openniStreamApp::setup()
{

    mFont = Font( "Times New Roman", 12.0f );
    V::OpenNIDeviceManager::USE_THREAD = false;
	_manager = V::OpenNIDeviceManager::InstancePtr();
    
    //    if( getArgs().size() > 1 )
    //	{
    //        //		nRetVal = m_Context.Init();
    //        //		CHECK_RC( nRetVal, "Init" );
    //        //		nRetVal = m_Context.OpenFileRecording( getArgs()[1] );
    //        //		if( nRetVal != XN_STATUS_OK ){
    //        //			printf( "Can't open recording %s: %s\n", argv[1], xnGetStatusString( nRetVal ) );
    //        //			return 1;
    //        //		}
    
    
    const XnChar* filename = "/Users/eight/repos/openFrameworks/apps/eight/openNI2Example/bin/data/20120811-164159-dev0.oni";//20120505-160636.oni";

    _manager->createDevice(filename/*getArgs()[1]*/, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH | V::NODE_TYPE_USER | V::NODE_TYPE_SCENE );
    //	} else 
    {
    //_manager->createDevices( 1, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH | V::NODE_TYPE_SCENE | V::NODE_TYPE_USER );
    }
	_device0 = _manager->getDevice( 0 );
    _device0->setDepthShiftMul( 3 );
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Can't find a kinect device\n" );
        quit();
        shutdown();
	}
    _device0->addListener( this );

	pixels = new uint16_t[ KINECT_DEPTH_SIZE ];
    previousPixels  = new uint16_t [KINECT_DEPTH_SIZE];
    pixelDiff = new uint16_t[KINECT_DEPTH_SIZE];
    std::copy(pixels, pixels+KINECT_DEPTH_SIZE, previousPixels);

	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT );

	_manager->start();
    
    host = "localhost";//188.32.31.209";//23.21.47.55";//;
	port = 3000;
//	sender.setup(host, port);
//    server = new srl::Server(io_service, 3333);
    
//    queueToServer = new ph::ConcurrentQueue<uint16_t*>();
//    std::shared_ptr<std::thread>(new boost::thread(boost::bind(&openniStreamApp::threadLoop, this)));
    queueToServerV =  new ph::ConcurrentQueue<PixelEntry<uint16_t> >();
    std::shared_ptr<std::thread>(new boost::thread(boost::bind(&openniStreamApp::threadLoopV, this)));
    if (!running) running = true;
    
    originalDepthSurface = Surface16u(KINECT_DEPTH_WIDTH,KINECT_DEPTH_HEIGHT,false, ImageIo::Y);
}



void openniStreamApp::mouseDown( MouseEvent event )
{
}


void openniStreamApp::update()
{
    if( !V::OpenNIDeviceManager::USE_THREAD )
    {
        _manager->update();
    }
    
	// Update textures
	mColorTex = getColorImage();
	mDepthTex = getDepthImage();
    

	// Uses manager to handle users.
    for( std::map<int, gl::Texture>::iterator it=mUsersTexMap.begin(); it != mUsersTexMap.end(); ++it )
    {
        it->second = getUserImage( it->first );
        
    }
    
    if( _manager->hasUsers() )
	{
        //serializeUser();
        queueUser();
        //queueToServer->push(pixels);
	}
}

void openniStreamApp::queueUser(){
    std::transform(pixels,pixels+KINECT_DEPTH_SIZE,previousPixels,pixelDiff,std::minus<uint16_t>());

//    ImageSourceRef ref(new ImageSourceKinectDepth( pixels, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT )) ;
//    originalDepthSurface = Surface16u(ref);
//    memcpy(originalDepthSurface.getData(), pixels, KINECT_DEPTH_SIZE * sizeof(uint16_t));
//	cv::Mat originalDepthMat( toOcv(Channel16u(originalDepthSurface)) );
//
//    uint calcScale = 1;
//	int scaledWidth = originalDepthSurface.getWidth() / calcScale;
//	int scaledHeight = originalDepthSurface.getHeight() / calcScale; 
//	cv::Mat resizedDepthMat( scaledWidth, scaledHeight, CV_16UC1 );
//	cv::resize( originalDepthMat, resizedDepthMat, resizedDepthMat.size(), 0, 0, cv::INTER_LINEAR);
//    resizedSurface = fromOcv(resizedDepthMat);

    queueToServerV->push(PixelEntry<uint16_t>(pixels, KINECT_DEPTH_SIZE));
    std::copy(pixels, pixels+KINECT_DEPTH_SIZE, previousPixels);
}

void openniStreamApp::draw()
{
    endTime = startTime;
    startTime = getElapsedSeconds();
    frameTime = startTime - endTime;
    //app::console() << frameTime << std::endl;
    
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 


	gl::setMatricesWindow( WIDTH, HEIGHT );

	gl::disableDepthWrite();
	gl::disableDepthRead();

	int sx = 320/2;
	int sy = 240/2;
	int xoff = 10;
	int yoff = 10;
	//glEnable( GL_TEXTURE_2D );
	gl::color( cinder::ColorA(1, 1, 1, 1) );
	gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
	gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );


    // Render all user textures
    int xpos = 5;
    int ypos = sy+10;
    for( std::map<int, gl::Texture>::iterator it=mUsersTexMap.begin(); it != mUsersTexMap.end();++it )
    {
        //int id = it->first;
        gl::Texture tex = it->second; //
    
        gl::draw( tex, Rectf(xpos, ypos, xpos+sx, ypos+sy) );
        xpos += sx;
        if( xpos > (sx+10) )
        {
            xpos = 5;
            ypos += sy+10;
        }
    }
    

	if( _manager->hasUsers() )
	{
		gl::disable( GL_TEXTURE_2D );
		// Render skeleton if available
		_manager->renderJoints( WIDTH, HEIGHT, 0, 3, false );
	}
    
    skeletonRenderer.draw(300, 300, 100.0f, 10.0f, false);
    std::ostringstream frmrtStrm;
    frmrtStrm<< getAverageFps();
    gl::drawString( "Framerate: " + frmrtStrm.str(), Vec2f( 10.0f, 10.0f ), Color::white(), mFont );
}

void openniStreamApp::serializeUser(){
    //string_inflate_deflate();

    serializeSkeleton();
    serializeUserPixels();
    
    osc::Message message;
    message.addIntArg(_manager->getUser(1)->getId());
    message.addIntArg(lenSkel);
    message.addStringArg(skelStrZ);
    message.addIntArg(lenUsrPx);
    int cut = pixStrZ.length() / 20;
//    message.addStringArg(pixStrZ.substr(0,cut));
    //std::cout<<pixStrZ.substr(0,cut).size()<<std::endl;
    message.setAddress("/newyork/osc/1");
    message.setRemoteEndpoint(host, port);
    sender.sendMessage(message);
}


void openniStreamApp::serializeUserPixels(){
    int count = 0;
    uint16_t *buff = pixelDiff;
    std::ostringstream pixStrStream;
    diffSize = 0;
    while( count < KINECT_DEPTH_SIZE ){ 
        if ((*buff)>0){
            pixStrStream <<*buff<<"|";
            diffSize++;
        }
            buff++;
            count ++;
    }
    pixStr = pixStrStream.str();
    lenUsrPx = pixStr.size();
    unsigned long dsize = lenUsrPx + (lenUsrPx * 0.1f) + 16;
	char * destination = new char[dsize];
    string_compress(pixStr, dsize, destination);
    pixStrZ = std::string(destination, dsize);
    //findLocations(pixStr,'\0');
    std::replace( pixStrZ.begin(), pixStrZ.end()-1, '\0', '\n');
    //findLocations(pixStrZ,'\0');
    //std::cout<<pixStrZ.size()<<" "<< pixStrZ.find('\0')<<std::endl;
    
    //std::cout<<pixStrZ<<std::endl;
    
    delete [] destination;
    destination = NULL;
    //pixStr = string_uncompress(pixStrZ,lenUsrPx);

    std::transform(pixels,pixels+KINECT_DEPTH_SIZE,previousPixels,pixelDiff,std::minus<uint16_t>());
    std::copy(pixels, pixels+KINECT_DEPTH_SIZE, previousPixels);
}


void openniStreamApp::serializeSkeleton(){
    V::OpenNIBoneList boneList = _manager->getUser(1)->getBoneList();
    std::ostringstream skeleton;
    for(std::vector<V::OpenNIBone*>::iterator it = boneList.begin(); it != boneList.end(); ++it) {
        //std::cout << (*it)->id<<" "<<(*it)->position[0]<<" "<<(*it)->position[1]<<" "<<(*it)->position[2]<<" "<<std::endl;
        skeleton <<(*it)->idd<<"|"<<(*it)->positionProjective[0]<<"|"<<(*it)->positionProjective[1]<<"|"<<(*it)->positionProjective[2]<<"\n";
    }
    skelStr = skeleton.str();
    lenSkel = skelStr.length();
    
    unsigned long dsize = lenSkel + (lenSkel * 0.1f) + 16;
	char * destination = new char[dsize];
    string_compress(skelStr, dsize, destination);
    skelStrZ = std::string(destination, dsize);
    
    delete [] destination;
    
    destination = NULL;

    //    std::cout<<skelStr<<" "<<skelStr.size()<<std::endl;
    //string_uncompress(skelStr,len);
}

void openniStreamApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
		this->quit();
		this->shutdown();
	}
}

void openniStreamApp::shutdown(){

}


void openniStreamApp::onNewUser( V::UserEvent event )
{
	app::console() << "New User Added With ID: " << event.mId << std::endl;
    mUsersTexMap.insert( std::make_pair( event.mId, gl::Texture(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT) ) );
}


void openniStreamApp::onLostUser( V::UserEvent event )
{
	app::console() << "User Lost With ID: " << event.mId << std::endl;
    mUsersTexMap.erase( event.mId );
}


CINDER_APP_BASIC( openniStreamApp, RendererGl )
