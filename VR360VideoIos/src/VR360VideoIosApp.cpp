#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/qtime/QuickTimeGl.h"

#include "cinder/MotionManager.h"

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
    
    CameraPersp         _camera;
    
    gl::TextureRef      _texture;
    gl::BatchRef        _sphere;
    qtime::MovieGlRef   _video;
};

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
    if ( _video->checkNewFrame() ) _texture = _video->getTexture().loadTopDown();

    _camera.setOrientation( MotionManager::getRotation( getOrientation() ) );
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
}

CINDER_APP( VR360VideoIosApp, RendererGl );
