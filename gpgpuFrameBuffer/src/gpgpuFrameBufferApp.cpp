#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "gpgpuFrameBuffer/gpGpuFrameBuffer.h"

#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gpgpuFrameBufferApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void update() override;
    void draw() override;
    
    gl::GlslProgRef     shader, shaderRefraction;
    
    
    gpGpuFrameBufferRef mBuffer;
    
    gl::TextureRef      mTexture;
    
    vec2                mMousePos;
    bool                mMouseDown;
    
};

void gpgpuFrameBufferApp::setup()
{
    mMouseDown = false;
    mMousePos = vec2(0.0);
    mBuffer = gpGpuFrameBuffer::create( getWindowWidth(), getWindowHeight() );
    
    try {
        shader = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/passThru.vert"))
                                      .fragment(loadAsset("shaders/gpgpu.frag")));
        shaderRefraction = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/passThru.vert"))
                                                .fragment(loadAsset("shaders/refraction.frag")));
    }
    catch (gl::GlslProgCompileExc ex) {
        console() << "GLSL Error: " << ex.what() << endl;
        
    }
    catch (gl::GlslNullProgramExc ex) {
        console() << "GLSL Error: " << ex.what() << endl;
        ci::app::App::get()->quit();
    }
    catch (...) {
        console() << "Unknown GLSL Error" << endl;
        ci::app::App::get()->quit();
    }
    
    mTexture = gl::Texture::create( loadImage( loadAsset("texture.jpg") ) );
    
}

void gpgpuFrameBufferApp::mouseDown( MouseEvent event )
{
    mMouseDown = true;
    mMousePos = event.getPos();
    
}

void gpgpuFrameBufferApp::mouseUp( MouseEvent event )
{
    mMouseDown = false;
    
}

void gpgpuFrameBufferApp::mouseDrag( MouseEvent event )
{
    mMousePos = event.getPos();
    
}

void gpgpuFrameBufferApp::update()
{
    //Using bind call
    
    mBuffer->bindBuffer();
    gl::viewport(mBuffer->getSize());
    gl::setMatricesWindow( mBuffer->getSize() );
    gl::clear();
    
    mBuffer->bindTexture();
    {
        shader->bind();
        shader->uniform( "pixel", vec2(1.0f)/vec2(mBuffer->getSize()) );
        shader->uniform( "texBuffer", 0 );
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0f),mBuffer->getSize()));
    }
    
    mBuffer->unbindTexture();
    
    if(mMouseDown)
    {
        gl::getStockShader(gl::ShaderDef().color())->bind();
        
        //Red channel will host the injected force from mouse position
        gl::color( ColorAf( 1.0f, 0.0f, 0.0f, 1.0f ) );
        gl::drawSolidCircle( ( mMousePos ), 5.0f );
        gl::color( Color::white() );
    }
    
    mBuffer->unbindBuffer(true);
    
}

void gpgpuFrameBufferApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    gl::viewport(getWindowSize());
    gl::setMatricesWindow( getWindowSize() );
    
    gl::enableAdditiveBlending();
    
    //using scoped
    {
        gl::ScopedTextureBind t0(mBuffer->getTexture(), 0);
        gl::ScopedTextureBind t1(mTexture, 1);
        
        gl::ScopedGlslProg sh(shaderRefraction);
        shaderRefraction->uniform( "pixel", vec2(1.0f)/vec2(mBuffer->getSize()) );
        shaderRefraction->uniform( "texBuffer", 0 );
        shaderRefraction->uniform( "texRefract", 1 );
        
        gl::drawSolidRect(getWindowBounds());
    }
    
    mBuffer->draw();
    

    
}

CINDER_APP( gpgpuFrameBufferApp, RendererGl )
