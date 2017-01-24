#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/qtime/QuickTimeGl.h"

#include "cinder/MotionManager.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const float kTargetSize = 2.0f;
static const float kTargetDistance = 4.0f;
static const float kHorizontalSize = kTargetSize * 4.0f;

class VR360VideoIosApp : public App
{
public:
    void                setup           ( ) override;
    void                update          ( ) override;
    void                draw            ( ) override;
    void                resize          ( ) override;
private:
    void                createGeometry();
    void                loadGeomSource( const geom::Source &source );
    void                createShaders();
    void                nextShape();
    void                straightenViewAnimated();
    void                turnViewForCylinderAnimated();
    void                renderToServerFbo();

    CameraPersp         _camera;
    
    gl::TextureRef      _texture;
    gl::BatchRef        _sphere;
    qtime::MovieGlRef   _video;
    const std::vector<std::string> mShapeNames = {"plane", "cylinder", "sphere"};
    enum Shapes {Plane, Cylinder, Sphere};
    enum Transition {PlaneCylinder, CylinderPlane};
    vector<gl::GlslProgRef> mShapeShaders;
    int                 mCurrntShape;
    int                 mNextShape;
    int                 mTransition;
    int                 mTransitionTime;
    AxisAlignedBox      mBbox;
    vec3				mCameraCOI;
    gl::BatchRef		mPrimitive;
    Anim<float>         mMix;
    Anim<quat>          mQuatAnim, mRotateQuatAnim;
    
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    // number of frequency bands of our spectrum
    const int    kBands = 1024;
    const int    kHistory = 128;
    const int    kWidth = 256;
    const int    kHeight = 256;
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    gl::TextureRef		mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;
    uint32_t			mOffset;
    
    vector<float>		mMagSpectrum;
    float               mVolume;
    

};

void VR360VideoIosApp::createGeometry()
{
    auto plane = geom::Plane().subdivisions( ivec2( 100, 100 ) );
    
    loadGeomSource( geom::Plane( plane ) );
}

void VR360VideoIosApp::loadGeomSource( const geom::Source &source )
{
    // The purpose of the TriMesh is to capture a bounding box; without that need we could just instantiate the Batch directly using primitive
    TriMesh::Format fmt = TriMesh::Format().positions().normals().texCoords();
    
    TriMesh mesh( source, fmt );
    mBbox = mesh.calcBoundingBox();
    mCameraCOI = mesh.calcBoundingBox().getCenter();
    
    mPrimitive = gl::Batch::create( mesh, mShapeShaders[mTransition] );
}

void VR360VideoIosApp::createShaders()
{
    try {
        mShapeShaders.push_back( gl::GlslProg::create( loadAsset( "shaders/plane-cylinder_es2.vert" ), loadAsset( "shaders/spectrum_es2.frag" ) ));
        mShapeShaders.push_back(gl::GlslProg::create( loadAsset( "shaders/cylinder-sphere_es2.vert" ), loadAsset( "shaders/spectrum_es2.frag" ) ));
        mShapeShaders.push_back(gl::GlslProg::create( loadAsset( "shaders/sphere-plane_es2.vert" ), loadAsset( "shaders/spectrum_es2.frag" ) ));
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading phong shader: " << exc.what() );
    }
}

void VR360VideoIosApp::nextShape(){
    float nextMix = mMix == 1.0f ? 0.0f : 1.0f;
    
    if (mCurrntShape == Plane && mNextShape == Cylinder)  {
        mTransition = 0;
    } else if ( mCurrntShape == Cylinder &&  mNextShape == Plane) {
        mMix = 0.0f;
        nextMix = 1.0f;
        cout<< mMix << " " <<nextMix<<endl;
        mTransition = 0;
    } else if (mCurrntShape == Cylinder && mNextShape == Sphere){
        mTransition = 1;
        mMix = 1.0f;
        nextMix = 0.0f;
    } else if (mCurrntShape == Sphere &&  mNextShape == Cylinder) {
        mTransition = 1;
    } else if (mCurrntShape == Plane &&  mNextShape == Sphere) {
        mTransition = 2;
    } else if (mCurrntShape == Sphere && mNextShape == Plane) {
        mTransition = 2;
    }
    
    mPrimitive->replaceGlslProg(mShapeShaders[mTransition]);
    
    if (mNextShape == Plane) {
        straightenViewAnimated();
    }
    
    if (mNextShape == Cylinder) {
        turnViewForCylinderAnimated();
    }
    
    timeline().appendTo( &mMix, nextMix, mTransitionTime, EaseInOutQuad() ).finishFn([this] { mCurrntShape = mNextShape; });
}

void VR360VideoIosApp::straightenViewAnimated()
{
    timeline().appendTo( &mQuatAnim, quat(), 5, EaseInOutQuad() );
}

void VR360VideoIosApp::turnViewForCylinderAnimated()
{
    vec3 normal = vec3( 0, 0, 1 );
    quat cylinderQuat =  angleAxis((float) M_PI_2, normalize( normal ) );
    timeline().appendTo( &mQuatAnim, cylinderQuat, 5, EaseInOutQuad() );
}

void VR360VideoIosApp::setup()
{
    getSignalSupportedOrientations().connect( [] { return InterfaceOrientation::All; } );
    MotionManager::enable( 60.0f, cinder::MotionManager::SensorMode::Gyroscope );
    gl::enableDepth();
    
    _sphere = gl::Batch::create ( geom::Sphere().subdivisions(64).radius(32), gl::getStockShader( gl::ShaderDef().texture() ) );
    
    _video = qtime::MovieGl::create( getAssetPath( "BVGG6623.mp4" ) );
    _video->setLoop();
    _video->play();

    _camera.setEyePoint( vec3( 0 ) );

    _texture = gl::Texture::create(loadImage(loadAsset("insta.png")));
    
    // Load and compile the shaders.
    createShaders();
    
    // start from current position
    mMix = 0.f;
    
    mCurrntShape = Sphere;
    mNextShape = Cylinder;
    mTransition = 1;
    createGeometry();
    
    nextShape();
    // Enable the depth buffer.
    gl::enableDepthRead();
    gl::enableDepthWrite();

    auto ctx = audio::Context::master();

    mInputDeviceNode = ctx->createInputDeviceNode();
    
    cout<< "Using " << mInputDeviceNode->getDevice() -> getName() << " audio input" <<endl;
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    mInputDeviceNode->enable();
    //ctx->enable();
    
//    mChannelLeft = Channel32f(kBands, kHistory);
//    mChannelRight = Channel32f(kBands, kHistory);
//    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
//    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );
//#if !defined( CINDER_COCOA_TOUCH )
//    mTextureFormat.setWrapS( GL_CLAMP_TO_BORDER );
//#endif
//    mTextureFormat.setWrapT( GL_REPEAT );
//    mTextureFormat.setMinFilter( GL_LINEAR );
//    mTextureFormat.setMagFilter( GL_LINEAR );

}

void VR360VideoIosApp::resize()
{
    bool isPortrait = getWindowHeight() > getWindowWidth();
    float adjustedViewWidth = ( isPortrait ? kHorizontalSize : (kHorizontalSize / kTargetSize) * getWindowAspectRatio() );
    float theta = 2.0f * math<float>::atan2( adjustedViewWidth / 2.0f, kTargetDistance );
    _camera.setPerspective( toDegrees( theta ) , getWindowAspectRatio(), 1, 1000 );
}

void VR360VideoIosApp::update ( )
{
    //if ( _video->checkNewFrame() ) _texture = _video->getTexture();//.loadTopDown();

    _camera.setOrientation( MotionManager::getRotation( getOrientation() ) );
    return;
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();

    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);
    
    // increment texture offset
    mOffset = (mOffset+1) % kHistory;
    
    mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
    mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
}

void VR360VideoIosApp::renderToServerFbo()
{
    if (!mTextureLeft || !mTextureRight || !_texture) return;
    gl::clear( Color::black() );
    gl::enableAlphaBlending();
    gl::setMatrices( _camera );

    gl::setDefaultShaderVars();

    gl::ScopedTextureBind scopedTextureBind( _texture );
    
    gl::rotate( mQuatAnim );
    gl::rotate(mRotateQuatAnim);
    
    // Draw the primitive.
    gl::ScopedColor colorScope( Color( 0.7f, 0.5f, 0.3f ) );
    
    float off = (mOffset / float(kHistory) - 0.5) * 2.0f;
    gl::GlslProgRef shader = mPrimitive->getGlslProg();
    shader->uniform("uTexOffset", off);
    shader->uniform("uMix", mMix);
    shader->uniform("resolution", 0.25f*(float)kWidth);
    shader->uniform("uTex0",0);
    shader->uniform("uLeftTex", 1);
    shader->uniform("uRightTex", 2);
    shader->uniform("uVolume", (1.0f + mVolume * mMonitorSpectralNode->getVolume()));
    
    gl::ScopedTextureBind texLeft( mTextureLeft, 1 );
    gl::ScopedTextureBind texRight( mTextureRight, 2 );
    
    gl::ScopedBlendAdditive blend;
    mPrimitive->draw();
    
}


void VR360VideoIosApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::setMatrices(_camera);
    if ( _texture )
    {
        gl::ScopedTextureBind tex0 ( _texture );
        _sphere->draw();
    }
    renderToServerFbo();
}

CINDER_APP( VR360VideoIosApp, RendererGl );
