#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "Resources.h"
#include "VOpenNIHeaders.h"
#include "KinectTextures.h"
#include "PersistentParams.h"
#include <fstream>
#include "ObjExporter.h"

static const int VBO_X_RES  = 640;
static const int VBO_Y_RES  = 480;

static const int KINECT_COLOR_WIDTH = 640;	//1280;
static const int KINECT_COLOR_HEIGHT = 480;	//1024;
static const int KINECT_COLOR_FPS = 30;	//15;
static const int KINECT_DEPTH_WIDTH = 640;
static const int KINECT_DEPTH_HEIGHT = 480;
static const int KINECT_DEPTH_SIZE = KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH;
static const int KINECT_DEPTH_FPS = 30;

using namespace ci;
using namespace ci::app;
using namespace std;



class openniMesh : public AppBasic, V::UserListener {
  public:
	void prepareSettings( Settings* settings );
    void setup();
    void shutdown();

    void mouseDrag(MouseEvent e);
    void keyDown(KeyEvent e);
    void mouseMove(MouseEvent e);
    void mouseUp(MouseEvent event);
	void update();
	void draw();
	
	// PARAMS
	PersistentParams	mParams;
	
	// CAMERA
	CameraPersp		mCam;
    cinder::Quatf	mSceneRotation;
    Vec3f           rotate, translate;
    Vec3f           rotate2, translate2;
	Vec3f			mEye, mCenter, mUp;
	float			mCameraDistance;
    ci::Vec3f		mLookAt;
	ci::Vec3f		mRotation;

	// VBO AND SHADER
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mShader;

    Font mFont;
    
    ~openniMesh();
    
private:
    ExportQueueRef                      exportQueue;
    std::shared_ptr<ObjExporter>        objExporter;
    bool                                next, continuos;
	ci::Vec3f							mScale;
    float                               floorDistMax, floorDistMax2;
    bool                                startWriting;
    bool                                startWritingMerged;
    bool                                mouseDragging;
    
    XnPlane3D plane0, plane1;
    Matrix44f transform;
    std::vector<Vec3f> mPositions;

    boost::unordered_map<Vertex, size_t, VertexHash> vrtxMap;
    boost::unordered_map<Vertex, size_t, VertexHash> vrtxMap2;
	std::vector<Vertex> indV, indV2;
    std::shared_ptr<std::thread> mServerThreadRef;

    void threadLoop();
    bool running;
    
	void createPointVbo();
    void createTriangleVbo();
    
    void exportPcdCloud(string filename, XnPoint3D* realWorld, size_t n);
    void exportPcdCloud(string filename, XnPoint3D* realWorld1, XnPoint3D* realWorld2, size_t n1, size_t n2);
    void exportTxtCloud(string filename, XnPoint3D* realWorld1, XnPoint3D* realWorld2, size_t n1, size_t n2);

    std::string getFileNameSuffix(size_t counter);
    
    void transformPointCloud(XnPoint3D* realWorld);
    
    bool show1, show2;
    
    // keep track of the mouse
	Vec2i		mMousePos;
    
    V::OpenNIDeviceManager*	_manager0;
    V::OpenNIDeviceManager*	_manager1;
	V::OpenNIDevice::Ref	_device0;
    V::OpenNIDevice::Ref	_device1;
    size_t                  fileFrameCounter, frameCounter, frameCounter2;
    
    Vec3f                   cutOff, cutOff2;
    uint16_t*				pixels;

    XnPoint3D*              transformedProjective;

    const XnPoint3D*        transformedRealWorld;
    XnPoint3D*              mergedRealWorld;

    gl::Texture				mColorTex, mColorTex2;
	gl::Texture				mDepthTex, mDepthTex2;
    
    Surface tmp;

    std::map<int, gl::Texture> mUsersTexMap;
    bool    showUser;

	void setupWriter();
    
    void onNewUser( V::UserEvent event );
	void onLostUser( V::UserEvent event );
    
    void drawTexture(gl::Texture mDepthTex, Vec3f rotate, Vec3f translate);
    
    void trace( const string & message);
    
    bool performPicking( Vec3f *pickedPoint, Vec3f *pickedNormal );
    
    float floorDistance( Vec3f P, XnPlane3D plane);
    
	ImageSourceRef getColorImage(V::OpenNIDevice::Ref dev)
	{
		// register a reference to the active buffer
		uint8_t *activeColor = dev->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}
    
	ImageSourceRef getUserImage( int id )
	{
		_device0->getLabelMap( id, pixels );
		return ImageSourceRef( new ImageSourceKinectDepth( pixels, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
	ImageSourceRef getDepthImage(V::OpenNIDevice::Ref dev, XnPlane3D plane, float floorDistMax, Vec3f cutOff)
	{
		// register a reference to the active buffer
        XnPoint3D* realWorld = dev->getDepthMapRealWorld();
        uint16_t *activeDepth = dev->getDepthMap();

        for (size_t i = 0; i < KINECT_DEPTH_SIZE; i++){
            float floor = floorDistance(Vec3f(realWorld[i].X,realWorld[i].Y,realWorld[i].Z), plane);
            if (floor>floorDistMax || realWorld[i].Z>cutOff.z){
                activeDepth[i]=0;
                realWorld[i].X = realWorld[i].Y = realWorld[i].Z = 0;
            }
        }

		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
};

void openniMesh::threadLoop()
{
    while (running) {
        try {
            boost::shared_ptr<ObjExporter> server = boost::shared_ptr<ObjExporter>(new ObjExporter("eliot2", exportQueue, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT));
            server.get()->run();
        }
        catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

void openniMesh::keyDown(KeyEvent e){
    if (e.getCode() == KeyEvent::KEY_RIGHT){
        next = !next;
    }
    
    if (e.getCode() == KeyEvent::KEY_c){
        continuos = !continuos;
    }

    if (e.getCode() == KeyEvent::KEY_w){
        startWriting = !startWriting;
    }

    if (e.getCode() == KeyEvent::KEY_m){
        startWritingMerged = !startWritingMerged;
    }
}


void openniMesh::mouseMove( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
}

openniMesh::~openniMesh(){
    mParams.save();

    delete [] pixels;

    if (exportQueue){
    	exportQueue->empty();
    }
}

void openniMesh::shutdown()
{
	running = false;
//	mSurfaces->cancel();

    if (mServerThreadRef){
//        mServerThreadRef->interrupt();
        mServerThreadRef->join();
    }
}

void openniMesh::setup()
{
    setFrameRate(30.0);
    
    objExporter = std::shared_ptr<ObjExporter>(new ObjExporter("eliot2", KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT));
    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.erase (vec.begin()+0 );
    for (std::vector<int>::iterator i = vec.begin(); i< vec.end(); i++)
        std::cout<< *i<<std::endl;
    
    mouseDragging = false;

    show1 = show2 = true;
    startWritingMerged = false;
    fileFrameCounter = frameCounter = frameCounter2 = 0;
    
    exportQueue = ExportQueueRef(new ExportQueue());

    PersistentParams::load( std::string(getenv("HOME")) + "/.opennimeshparams" );
    
    mParams = PersistentParams( "OpenniMesh", Vec2i( 200, 500 ) );

    mParams.addPersistentParam ( "Max X", &cutOff.x, (float) 1);
    mParams.addPersistentParam ( "Max Y", &cutOff.y, (float) 1);
    mParams.addPersistentParam ( "Max Z", &cutOff.z, (float) 1);

    mParams.addPersistentParam ( "Max2 X", &cutOff2.x, (float) 1);
    mParams.addPersistentParam ( "Max2 Y", &cutOff2.y, (float) 1);
    mParams.addPersistentParam ( "Max2 Z", &cutOff2.z, (float) 1);
//	mParams.addPersistentParam ( "Rotate X", &rotate.x, (float) 1);
//	mParams.addPersistentParam ( "Rotate Y", &rotate.y, (float) 1);
//	mParams.addPersistentParam ( "Rotate Z", &rotate.z, (float) 1);
//	mParams.addPersistentParam ( "Translate X", &translate.x, (float) 1);
//	mParams.addPersistentParam ( "Translate Y", &translate.y, (float) 1);
//	mParams.addPersistentParam ( "Translate Z", &translate.z, (float) 1);
    mParams.addParam ("Show 1 ", &show1, "", false);
//    mParams.addPersistentParam ( "Rotate X2", &rotate2.x, (float) 1);
//	mParams.addPersistentParam ( "Rotate Y2", &rotate2.y, (float) 1);
//	mParams.addPersistentParam ( "Rotate Z2", &rotate2.z, (float) 1);
//	mParams.addPersistentParam ( "Translate X2", &translate2.x, (float) 1);
//	mParams.addPersistentParam ( "Translate Y2", &translate2.y, (float) 1);
//	mParams.addPersistentParam ( "Translate Z2", &translate2.z, (float) 1);
    mParams.addPersistentParam ("Floor ", &floorDistMax, 1);
    mParams.addPersistentParam ("Floor2 ", &floorDistMax2, 1);
    mParams.addParam ("Show 2 ", &show2, "", false);

	mParams.addPersistentParam ( "Cam Distance", &mCameraDistance, 1.0f, "min=100.0 max=5000.0 step=100.0 keyIncr=D keyDecr=d" );
    mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
    mParams.addParam ( "User?", &showUser, "");


    plane0.ptPoint.X = 0;
    plane0.ptPoint.Y = -1319.92;
    plane0.ptPoint.Z = 349.188;
    
    plane0.vNormal.X = 0.0143922;
    plane0.vNormal.Y = 0.973744;
    plane0.vNormal.Z = -0.227192;

    plane1.ptPoint.X = 0;
    plane1.ptPoint.Y = -1327.59;
    plane1.ptPoint.Z = 349.188;
    
    plane1.vNormal.X = -0.00213706;
    plane1.vNormal.Y = 0.987077;
    plane1.vNormal.Z = -0.160235;
    
	
	// SETUP CAMERA

	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
    
    //SETUP TRANSFORM
    transform = Matrix44f(-0.01, -0.2, -0.98, 4777.45, 0.07, 0.98, -0.2, 1142.70, 1.0, -0.07, 0.01, 4889.15, 0.0, 0.0, 0.0, 1.0, true);

    // SETUP OPENNI
    
    V::OpenNIDeviceManager::USE_THREAD = false;
	_manager0 = new V::OpenNIDeviceManager();
	_manager1 = new V::OpenNIDeviceManager();
    //20120811-151442-dev1.oni skripka/para
    //20120811-164159-dev1.oni  glavnoe
    //20120811-164159-dev0.oni
    const XnChar* filename1 = "/Users/vgusev/onis/20120811-164159-dev1.oni";// ashes
    
    //20120811-141214-calib-dev1.oni";//";//20120811-170349-dev1.oni; baraban
    const XnChar* filename0 = "/Users/vgusev/onis/20120811-164159-dev0.oni";
    //20120811-141213-calib-dev0.oni";//20120811-170349-dev1.oni; baraban

    _manager0->createDevice(filename0, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH, 0);
    _manager1->createDevice(filename1, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH, 0);

	_device0 = _manager0->getDevice( 0 );
    //_device0->setDepthShiftMul( 3 );
    _device0->setAlignWithDepthGenerator();

    
    _device1 = _manager1->getDevice( 0 );
    //_device1->setDepthShiftMul( 3 );
    _device1->setAlignWithDepthGenerator();
	if( !_device0 )
	{
		DEBUG_MESSAGE( "(App)  Can't find a kinect device\n" );
        quit();
        shutdown();
	}
    _device0->addListener(this);
    _device1->addListener(this);

    _manager1->setFrame(4, 0);
    
	pixels = new uint16_t[ KINECT_DEPTH_SIZE ];

	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT );

	mColorTex2 = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT );
	mDepthTex2 = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT );

	_manager0->start();
	_manager1->start();
    
    transformedProjective = new XnPoint3D[ KINECT_DEPTH_SIZE ];
    
    mergedRealWorld = new XnPoint3D[KINECT_DEPTH_SIZE];
	
	// SETUP VBO AND SHADER
//	createTriangleVbo();
    createPointVbo();
    try {
        mShader	= gl::GlslProg( loadResource( RES_VERT_ID ), loadResource( RES_FRAG_ID ) );
    } catch (cinder::Exception e) {
        trace(e.what());
    }
	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();
    
    mFont = Font( "Helvetica", 22.0f );
    
    next = false;
    
    continuos = false;
    
    //setupWriter();

    running = true;
    //mServerThreadRef = std::shared_ptr<std::thread>(new boost::thread(boost::bind(&openniMesh::threadLoop, this)));
}

void openniMesh::setupWriter(){
    fs::path path = getSaveFilePath();
	if( path.empty() )
		return; // user cancelled save
}

void openniMesh::update()
{

    //OPENNI
    if( !V::OpenNIDeviceManager::USE_THREAD ){
        if (mouseDragging){
            if (show1){
                float frame = float(mMousePos.x)/getWindowWidth();
                _manager0->setFrame(floor(_manager0->getTotalNumFrames(0) * frame), 0);
            }
            if (show2){
                float frame = float(mMousePos.x)/getWindowWidth();
                _manager1->setFrame(floor(_manager1->getTotalNumFrames(0) * frame), 0);
            }
        }
        else if (next || continuos){
        if (show1){
            _manager0->update();
            _device0->calcDepthImageRealWorld();
            
            frameCounter++;
        }
        if (show2){
            bool skip = (frameCounter2 % 35) == 0;
            if (!skip){
            _manager1->update();
            _device1->calcDepthImageRealWorld();
            }
            frameCounter2++;
        }
            cout <<"FRAMES: "<<frameCounter<< " "<<frameCounter2<<endl;
        if (show1){
            mColorTex = getColorImage(_device0);
            mDepthTex = getDepthImage(_device0, plane0, floorDistMax, cutOff);

//            if (startWriting){
//                exportPcdCloud("eliotkill0", _device0->getDepthMapRealWorld(), KINECT_DEPTH_SIZE);
//            }
        }
        if (show2){
            mColorTex2 = getColorImage(_device1);
            mDepthTex2 = getDepthImage(_device1, plane1, floorDistMax2, cutOff2);
            transformPointCloud(_device1->getDepthMapRealWorld());
        }
        
        if (show1 && show2 && startWritingMerged){
            objExporter->exportDepthToObj(_device0->getDepthMapRealWorld(), _device1->getDepthMapRealWorld(),Surface(mColorTex), Surface(mColorTex2));
            //exportQueue->push(ObjData(_device0->getDepthMapRealWorld(), _device1->getDepthMapRealWorld(),Surface(mColorTex), Surface(mColorTex2)) );
        }

        startWriting = false;
        next = false;
        }
    }
    
	mEye = Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
}

void openniMesh::transformPointCloud(XnPoint3D *realWorld){
    const Vec3f wrongPoint(4777.45, 1142.7, 4889.15);
    for (int i = 0; i < KINECT_DEPTH_SIZE; i++){
        Vec3f current = Vec3f(realWorld[i].X,realWorld[i].Y,realWorld[i].Z);
        if (current.lengthSquared() == 0) continue;
        Vec3f transformed = transform*current;
        realWorld[i].X = transformed.x;
        realWorld[i].Y = transformed.y;
        realWorld[i].Z = transformed.z;
    }
}

void openniMesh::drawTexture(gl::Texture mDepthTex, Vec3f rotate, Vec3f translate){
  	gl::pushMatrices();
    gl::scale( Vec3f( -1.0f, -1.0f, 1.0f ) );
    gl::rotate( mSceneRotation );
    Matrix44f transform;
    transform.rotate( Vec3f(toRadians(rotate.x), toRadians(rotate.y), toRadians(rotate.z)) );
    transform.translate( translate );
    
    gl::pushModelView();
    gl::multModelView( transform );
    
    mDepthTex.bind( 0 );
    
    mShader.bind();
    mShader.uniform("depthTex", 0 );
    
    gl::draw( mVboMesh );
    mShader.unbind();
    
	gl::popMatrices();
    gl::popModelView();
  
}

void openniMesh::draw(){
    
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );


    	mColorTex.setFlipped();
        gl::draw( mColorTex);//, Rectf(sx, yoff, xoff+sx*2, yoff+sy) );

        gl::pushMatrices();
        
        glTranslatef( 640, 10, 0.0f );
        
        mColorTex2.setFlipped();
        gl::draw( mColorTex2);//, Rectf( xoff+sx*2, yoff+2*sy, xoff+sx*3, yoff+2*sy));
        gl::popMatrices();


    if (show1){
        drawTexture(mDepthTex, rotate, translate);
    }
    if (show2){
        drawTexture(mDepthTex2, rotate2, translate2);
    }
    mParams.draw();
}

void openniMesh::mouseDrag(MouseEvent event)
{
    mMousePos = event.getPos();
    mouseDragging = true;
}

void openniMesh::mouseUp(MouseEvent event)
{
    mouseDragging = false;
}

void openniMesh::onNewUser( V::UserEvent event ) {
	app::console() << "New User Added With ID: " << event.mId << std::endl;
    //XnPlane3D floor = event.mDevice->GetFloor();
    //app::console()<<"FLOOR "<<floor.ptPoint.X<<" "<<floor.ptPoint.Y<<" "<<floor.ptPoint.Z<<" "<<floor.vNormal.X<<" "<<floor.vNormal.Y<<" "<<floor.vNormal.Z<<std::endl;
    mUsersTexMap.insert( std::make_pair( event.mId, gl::Texture(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT) ) );
}


void openniMesh::onLostUser( V::UserEvent event )
{
	app::console() << "User Lost With ID: " << event.mId << std::endl;
    mUsersTexMap.erase( event.mId );
}

#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm(v)    sqrt(dot(v,v))  // norm = length of vector
#define d(u,v)     norm(u-v)       // distance = norm of difference

float openniMesh::// pbase_Plane(): get base of perpendicular from point to a plane
//    Input:  P = a 3D point
//            PL = a plane with point V0 and normal n
//    Output: *B = base point on PL of perpendicular from P
//    Return: the distance from P to the plane PL

floorDistance( Vec3f P, XnPlane3D plane)
{
    float    sb, sn, sd;
    Vec3f normal = Vec3f(plane.vNormal.X,plane.vNormal.Y, plane.vNormal.Z);
    
    sn = -dot(normal, (P - normal));
    sd = dot(normal, normal);
    sb = sn / sd;
    
    Vec3f base = P + sb * normal;
    return d(P, base);
}


void openniMesh::createPointVbo()
{
	gl::VboMesh::Layout layout;
	
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticIndices();
    
	std::vector<Vec2f> texCoords;
	std::vector<uint32_t> indices;
	
	int numVertices = VBO_X_RES * VBO_Y_RES;
	int numShapes	= ( VBO_X_RES - 1 ) * ( VBO_Y_RES - 1 );
    
	mVboMesh		= gl::VboMesh( numVertices, numShapes, layout, GL_POINTS );
	
	for( int x=0; x<VBO_X_RES; ++x ){
		for( int y=0; y<VBO_Y_RES; ++y ){
			indices.push_back( x * VBO_Y_RES + y );
            
			float xPer	= x / (float)(VBO_X_RES-1);
			float yPer	= y / (float)(VBO_Y_RES-1);
			mPositions.push_back( Vec3f( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES, 0.0f ) );
			texCoords.push_back( Vec2f( xPer, yPer ) );
		}
	}
	
	mVboMesh.bufferPositions( mPositions );
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d(0, texCoords );
    
}

void openniMesh::createTriangleVbo(){
    
    // VBO dimensions as floats
    float heightf	= (float)VBO_Y_RES;
    float widthf	= (float)VBO_X_RES;
    
    // VBO data
    gl::VboMesh::Layout vboLayout;
    vector<uint32_t> vboIndices;
    vector<Vec3f> vboPositions;
    vector<Vec2f> vboTexCoords;
    
    // Set up VBO layout
    vboLayout.setStaticIndices();
    vboLayout.setStaticPositions();
    vboLayout.setStaticTexCoords2d();
    
    // Define corners of quad (two triangles)
    vector<Vec2f> quad;
    quad.push_back( Vec2f( 0.0f, 0.0f ) );
    quad.push_back( Vec2f( 0.0f, 1.0f ) );
    quad.push_back( Vec2f( 1.0f, 0.0f ) );
    quad.push_back( Vec2f( 1.0f, 0.0f ) );
    quad.push_back( Vec2f( 0.0f, 1.0f ) );
    quad.push_back( Vec2f( 1.0f, 1.0f ) );
    
    // Iterate through rows in the mesh
    for ( int32_t y = 0; y < VBO_Y_RES; y++ ) {
        // Iterate through the vectors in this row
        for ( int32_t x = 0; x < VBO_X_RES; x++ ) {
            // Get vertices as floats
            float xf = (float)x;
            float yf = (float)y;
            
            // Add position in world coordinates
            vboPositions.push_back( Vec3f( xf - widthf * 0.5f, yf - heightf * 0.5f, 0.0f ) );
            
            // Use percentage of the position for the texture coordinate
            vboTexCoords.push_back( Vec2f( xf / widthf, yf / heightf ) );
            
            // Do not add a quad to the bottom or right side of the mesh
            if ( x < VBO_X_RES && y < VBO_Y_RES ) {
                
                // Iterate through points in the quad to set indices
                for ( vector<Vec2f>::const_iterator vertIt = quad.begin(); vertIt != quad.end(); ++vertIt ) {
                    
                    // Get vertices as floats
                    xf = (float)x + vertIt->x;
                    yf = (float)y + vertIt->y;
                    
                    // Set the index of the vertex in the VBO so it is
                    // numbered left to right, top to bottom
                    vboIndices.push_back( (uint32_t)( xf + yf * widthf ) );
                }
            }
        }
    }
    
    // Build VBO
    mVboMesh = gl::VboMesh( vboPositions.size(), vboIndices.size(), vboLayout, GL_TRIANGLES );
    mVboMesh.bufferIndices( vboIndices );
    mVboMesh.bufferPositions( vboPositions );
    mVboMesh.bufferTexCoords2d( 0, vboTexCoords );
    mVboMesh.unbindBuffers();
    
    // Clean up
    vboIndices.clear();
    vboPositions.clear();
    vboTexCoords.clear();
}

void openniMesh::exportTxtCloud(string filePrefix, XnPoint3D* realWorld1, XnPoint3D* realWorld2, size_t n1, size_t n2) {

    std::string myPath = getHomeDirectory().string()+"eliot/"+filePrefix+getFileNameSuffix(fileFrameCounter)+".txt";
    std::ofstream oStream( myPath.c_str() );

    for(int i = 0; i < n1; i++) {
        oStream << i<<", "<<realWorld1[i].X << ", "<< realWorld1[i].Y << ", "<< realWorld1[i].Z<<endl;
	}
    
    for(int i = 0; i < n2; i++) {
        oStream << (i+n1) << ", " << realWorld2[i].X << ", "<< realWorld2[i].Y << ", "<< realWorld2[i].Z<<endl;
	}
    oStream.close();
    
    fileFrameCounter++;
}


void openniMesh::exportPcdCloud(string filename, XnPoint3D* realWorld1, XnPoint3D* realWorld2, size_t n1, size_t n2) {
    
    std::string myPath = getHomeDirectory().string()+filename+".pcd";
    
    std::ofstream oStream( myPath.c_str() );
    
    // write the string.
    oStream << "# .PCD v.7 - Point Cloud Data file format"<<endl;
    
    oStream << "VERSION .7"<<endl;
    oStream << "FIELDS x y z"<<endl;
    oStream << "SIZE 4 4 4"<<endl;
    oStream << "TYPE F F F"<<endl;
    oStream << "COUNT 1 1 1"<<endl;
    oStream << "WIDTH "<< KINECT_DEPTH_WIDTH <<endl;
    oStream << "HEIGHT "<<KINECT_DEPTH_HEIGHT*2<<endl;
    oStream << "VIEWPOINT 0 0 0 1 0 0 0"<<endl;
    oStream << "POINTS "<< (n1+n2) <<endl;
    oStream << "DATA ascii" << endl;
    
    for(int i = 0; i < KINECT_DEPTH_SIZE; i++) {
        oStream << realWorld1[i].X << " "<< realWorld1[i].Y << " "<< realWorld1[i].Z<<endl;
	}
    
    for(int i = 0; i < KINECT_DEPTH_SIZE; i++) {
        oStream << realWorld2[i].X << " "<< realWorld2[i].Y << " "<< realWorld2[i].Z<<endl;
	}
    oStream.close();
}


void openniMesh::exportPcdCloud(string filename, XnPoint3D* realWorld, size_t n) {
    
    std::string myPath = getHomeDirectory().string()+filename+".pcd";
    
    std::ofstream oStream( myPath.c_str() );
    
    // write the string.
    oStream << "# .PCD v.7 - Point Cloud Data file format"<<endl;
    
    oStream << "VERSION .7"<<endl;
    oStream << "FIELDS x y z"<<endl;
    oStream << "SIZE 4 4 4"<<endl;
    oStream << "TYPE F F F"<<endl;
    oStream << "COUNT 1 1 1"<<endl;
    oStream << "WIDTH "<< KINECT_DEPTH_WIDTH <<endl;
    oStream << "HEIGHT "<<KINECT_DEPTH_HEIGHT<<endl;
    oStream << "VIEWPOINT 0 0 0 1 0 0 0"<<endl;
    oStream << "POINTS "<< n <<endl;
    oStream << "DATA ascii" << endl;
    
    for(int i = 0; i < KINECT_DEPTH_SIZE; i++) {
		//if (cloud[i] != 0) {
        
        oStream << realWorld[i].X << " "<< realWorld[i].Y << " "<< realWorld[i].Z<<endl;
		//}
	}
    oStream.close();
}

std::string openniMesh::getFileNameSuffix(size_t counter){
    if (counter<10)
            return "000000"+boost::lexical_cast<string>(counter);
    if (counter<100)
            return "00000"+boost::lexical_cast<string>(counter);
    if (counter<1000)
            return "0000"+boost::lexical_cast<string>(counter);
    if (counter<10000)
            return "000"+boost::lexical_cast<string>(counter);
    if (counter<100000)
            return "00"+boost::lexical_cast<string>(counter);
    if (counter<1000000)
            return "0"+boost::lexical_cast<string>(counter);
    return boost::lexical_cast<string>(counter);
}

void openniMesh::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void openniMesh::trace( const string & message){
    console() << message << "\n";
}

bool openniMesh::performPicking( Vec3f *pickedPoint, Vec3f *pickedNormal )
{
	// generate a ray from the camera into our world
	float u = mMousePos.x / (float) getWindowWidth();
	float v = mMousePos.y / (float) getWindowHeight();
	// because OpenGL and Cinder use a coordinate system
	// where (0, 0) is in the LOWERleft corner, we have to flip the v-coordinate
	Ray ray = mCam.generateRay(u , 1.0f - v, mCam.getAspectRatio() );
    
	// draw the object space bounding box in yellow
    //	gl::color( Color(1, 1, 0) );
    //	gl::drawStrokedCube(mObjectBounds);
    
	// the coordinates of the bounding box are in object space, not world space,
	// so if the model was translated, rotated or scaled, the bounding box would not
	// reflect that.
	//
	// One solution would be to pass the transformation to the calcBoundingBox() function:
    //	AxisAlignedBox3f worldBoundsExact = mMesh.calcBoundingBox(mTransform);		// slow
    //
    //	// draw this transformed box in orange
    //	gl::color( Color(1, 0.5, 0) );
    //	gl::drawStrokedCube(worldBoundsExact);
	
	// But if you already have an object space bounding box, it's much faster to
	// approximate the world space bounding box like this:
    //	AxisAlignedBox3f worldBoundsApprox = mObjectBounds.transformed(mTransform);	// fast
    //
    //	// draw this transformed box in cyan
    //	gl::color( Color(0, 1, 1) );
    //	gl::drawStrokedCube(worldBoundsApprox);
    //
    //	// fast detection first - test against the bounding box itself
    //	if( ! worldBoundsExact.intersects(ray) )
    //		return false;
    
	// set initial distance to something far, far away
	float result = 1.0e6f;
    float distance = 0.0f;
    
	// traverse triangle list and find the picked triangle
    
    
    //gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
    for( int idx = 0; idx < mPositions.size(); ++idx ) {
        //std::cout<< (*(iter.getPositionPointer())<<std::endl;
        //if( ray.calcTriangleIntersection(v0, v1, v2, &distance) )
        //app::console() << mPositions[idx] << std::endl;
        
    }
    
    //	size_t polycount = vboMesh.get();
    //	float distance = 0.0f;
    //	for(size_t i=0;i<polycount;++i)
    //	{
    //		Vec3f v0, v1, v2;
    //		// get a single triangle from the mesh
    //		mMesh.getTriangleVertices(i, &v0, &v1, &v2);
    //
    //		// transform triangle to world space
    //		v0 = mTransform.transformPointAffine(v0);
    //		v1 = mTransform.transformPointAffine(v1);
    //		v2 = mTransform.transformPointAffine(v2);
    //
    //		// test to see if the ray intersects with this triangle
    //		if( ray.calcTriangleIntersection(v0, v1, v2, &distance) ) {
    //			// set our result to this if its closer than any intersection we've had so far
    //			if( distance < result ) {
    //				result = distance;
    //				// assuming this is the closest triangle, we'll set our normal
    //				// while we've got all the points handy
    //				*pickedNormal = ( v1 - v0 ).cross( v2 - v0 ).normalized();
    //			}
    //		}
    //	}
    
	// did we have a hit?
	if( distance > 0 ) {
		*pickedPoint = ray.calcPosition( result );
		return true;
	}
	else
		return false;
}




CINDER_APP_BASIC( openniMesh, RendererGl )
