#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Capture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// This sample shows a very basic use case for FBOs - it renders a spinning colored cube
// into an FBO, and uses that as a Texture onto the sides of a blue cube.
class FboCamBasicApp : public App {
public:
    void	setup() override;
    void	update() override;
    void	draw() override;
    
private:
    void			renderSceneToFbo();
    
    gl::FboRef			mFbo;
    mat4				mRotation;
    static const int	FBO_WIDTH = 256, FBO_HEIGHT = 256;
    
    CaptureRef			mCapture;
    gl::TextureRef		mCamTex;
};

void FboCamBasicApp::setup()
{
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    mFbo = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, format.colorTexture() );
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
    
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            cout <<"COMLETE"<<endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            cout << "INCOMPLETE"<<endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            cout << "MISSING"<<endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            cout << "DIMENSIONS"<<endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            cout << "UNSUPPORTED"<<endl;
            break;
        case 0x8219 :
            cout<<"HERNIA"  << endl;
        default:
            break;
    }
    
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        cout<<"failed to make complete framebuffer object " << status << endl;
    }
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mCapture = Capture::create( 320, 240 );
    mCapture->start();
}

// Render the color cube into the FBO
void FboCamBasicApp::renderSceneToFbo()
{
    if (!mCamTex) return;
    // this will restore the old framebuffer binding when we leave this function
    // on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
    // but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
    gl::ScopedFramebuffer fbScp( mFbo );
    // clear out the FBO with blue
    gl::clear( Color( 0.25, 0.5f, 1.0f ) );
    
    // setup the viewport to match the dimensions of the FBO
    gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
    
    // setup our camera to render the torus scene
    CameraPersp cam( mFbo->getWidth(), mFbo->getHeight(), 60.0f );
    cam.setPerspective( 60, mFbo->getAspectRatio(), 1, 1000 );
    cam.lookAt( vec3( 2.8f, 1.8f, -2.8f ), vec3( 0 ));
    gl::setMatrices( cam );
    
    // set the modelview matrix to reflect our current rotation
    gl::setModelMatrix( mRotation );

    gl::ScopedTextureBind b(mCamTex);
    
    // render the color cube
    gl::ScopedGlslProg shaderScp( gl::getStockShader( gl::ShaderDef().texture() ) );
    
    //Rectf centered = Rectf( mFbo->getBounds() ).getCenteredFit( getWindowBounds(), false );
    //gl::color( Color( 1.0f, 0.5f, 0.25f ) );
    //gl::drawSolidRect( centered );

    gl::drawCube( vec3( 0 ), vec3( 2.2f ) );
    //gl::color( Color::white() );
}

void FboCamBasicApp::update()
{
    // Rotate the torus by .06 radians around an arbitrary axis
    mRotation *= rotate( 0.06f, normalize( vec3( 0.16666f, 0.333333f, 0.666666f ) ) );

    if( mCapture && mCapture->checkNewFrame() ) {
        if( ! mCamTex ) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mCamTex = gl::Texture::create( *mCapture->getSurface(), gl::Texture::Format().loadTopDown() );
        }
        else {
            mCamTex->update( *mCapture->getSurface() );
        }
    }
    
    // render into our FBO
    renderSceneToFbo();
}

void FboCamBasicApp::draw()
{
    // clear the window to gray
    gl::clear( Color( 0.35f, 0.35f, 0.35f ) );
    
    // setup our camera to render the cube
    CameraPersp cam( getWindowWidth(), getWindowHeight(), 60.0f );
    cam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
    cam.lookAt( vec3( 2.6f, 1.6f, -2.6f ), vec3( 0 ) );
    gl::setMatrices( cam );
    
    // use the scene we rendered into the FBO as a texture
    mFbo->bindTexture();
    
    // draw a cube textured with the FBO
    {
        gl::ScopedGlslProg shaderScp( gl::getStockShader( gl::ShaderDef().texture() ) );
        gl::drawCube( vec3( 0 ), vec3( 2.2f ) );
        Rectf centered = Rectf( mFbo->getBounds() ).getCenteredFit( getWindowBounds(), false );
        //gl::drawSolidRect( centered );
    }
    
    // show the FBO color texture in the upper left corner
    gl::setMatricesWindow( toPixels( getWindowSize() ) );
    gl::draw( mFbo->getColorTexture(), Rectf( 0, 0, 128, 128 ) );
    // and draw the depth texture adjacent
    gl::draw( mFbo->getDepthTexture(), Rectf( 128, 0, 256, 128 ) );
}

CINDER_APP( FboCamBasicApp, RendererGl )
