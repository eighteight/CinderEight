/*
 Copyright (c) 2014, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/ImageIo.h"
//#include "cinder/MayaCamUI.h"
#include "cinder/Rand.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include "cinder/gl/Batch.h"
#include "cinder/app/RendererGl.h"

#include "Resources.h"
#include "cinder/Perlin.h"
#include "cinder/params/Params.h"
#include "cinder/Capture.h"

#define INPUT_DEVICE "Scarlett 2i2 USB"

using namespace ci;
using namespace ci::app;
using namespace std;

class AudioVisualizerApp : public App {
public:
    void prepareSettings( Settings* settings );
    
    void setup();
    void shutdown();
    void update();
    void draw();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseUp( MouseEvent event );
    void keyDown( KeyEvent event );
    void mouseWheel( MouseEvent event );
    void resize();
    
private:
    // width and height of our mesh
    static const int kWidth = 256;
    static const int kHeight = 256;
    
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    CameraPersp			mCamera;
//    MayaCamUI			mMayaCam;
    vector<gl::GlslProgRef>		mShader;
    int mShaderNum;
    gl::TextureRef			mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;

    uint32_t			mOffset;
    
    bool				mIsMouseDown;
    double				mMouseUpTime;
    double				mMouseUpDelay;
    
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
    float mVolumeMax;
    
    Perlin				mPerlin;
    glm::uint32              mPerlinMove;
    std::vector<vec3>      mVertices;
    


    float mFrameRate;
    bool  mAutomaticSwitch;
    ci::params::InterfaceGl		mParams;
    
    CaptureRef mCapture;
    gl::TextureRef mTextureRef;
    gl::BatchRef mBatch;
    
    gl::VboMeshRef vboMeshRef;

};

void AudioVisualizerApp::prepareSettings(Settings* settings)
{
    settings->setFullScreen(false);
    settings->setWindowSize(1280, 720);
}

void AudioVisualizerApp::mouseWheel( MouseEvent event )
{
	// Zoom in/out with mouse wheel
	vec3 eye = mCamera.getEyePoint();
	eye.z += event.getWheelIncrement();
	mCamera.setEyePoint( eye );
    cout<<eye<<endl;
}

void AudioVisualizerApp::setup()
{
    mCapture = Capture::create(  640, 480 );
    mCapture->start();
    
    
    mPerlin = Perlin( 4, 0 );
    mPerlinMove = 0;
    mFrameRate	= 0.0f;
    mParams = params::InterfaceGl( "Params", vec2( 200, 100 ) );
	mParams.addParam( "Frame rate",	&mFrameRate,"", true);
	mParams.addParam( "Shader",	&mShaderNum,"min=0 max=3 step=1", false);
    mParams.addParam( "Auto switch", &mAutomaticSwitch);
    
    auto ctx = audio::Context::master();
    std::cout << "Devices available: " << endl;
    for( const auto &dev : audio::Device::getInputDevices() ) {
        std::cout<<dev->getName() <<endl;
    }
    
    std::vector<audio::DeviceRef> devices = audio::Device::getInputDevices();
    const auto dev = audio::Device::findDeviceByName(INPUT_DEVICE);
    if (!dev){
        cout<<"Could not find " << INPUT_DEVICE << endl;
        mInputDeviceNode = ctx->createInputDeviceNode();
        cout<<"Using default input"<<endl;
    } else {
        mInputDeviceNode = ctx->createInputDeviceNode(dev);
    }
    
    mVolumeMax = 0.f;
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    //getWindow()->setTitle( mInputDeviceNode->getDevice()->getName() );
    
    // setup camera
//    mCamera.setPerspective(50.0f, 1.0f, 1.0f, 10000.0f);
//    mCamera.setEyePoint( vec3(-kWidth/2, kHeight/2, -kWidth/8) );
//    mCamera.setEyePoint( vec3(10239.3,7218.58,-7264.48));
    //mCamera.setCenterOfInterestPoint( vec3(kWidth*0.5f, -kHeight*0.5f, kWidth*0.5f) );
    
//    mCamera.setPerspective(60, getWindowAspectRatio(), .01, 1000);
//    mCamera.lookAt(vec3( 11.021,   -5.600,    7.338), vec3(-31.018,   25.341,  -13.460));
//    mCamera.setEyePoint(vec3(11.021,   -5.600,    7.338));
    
    mCamera.setPerspective(60.0f, getWindowAspectRatio(), .01, 1000);
    mCamera.setEyePoint( vec3(113.553,   80.420, -303.449));
//    mCamera.setCenterOfInterestPoint( vec3(-22.514,   19.082,  252.038) );
    
    // create channels from which we can construct our textures
    mChannelLeft = Channel32f(kBands, kHistory);
    mChannelRight = Channel32f(kBands, kHistory);
    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );
    
    // create texture format (wrap the y-axis, clamp the x-axis)
    mTextureFormat.setWrapS( GL_CLAMP_TO_BORDER );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );

    TriMesh mesh( TriMesh::Format().positions( 3 ).texCoords( 2 ).colors( 3 ) );
    for(size_t h=0;h<kHeight;++h)
    {
        for(size_t w=0;w<kWidth;++w)
        {
            // add polygon indices
            if(h < kHeight-1 && w < kWidth-1)
            {
                size_t offset = mVertices.size();
                
                mesh.appendTriangle( offset, offset+kWidth, offset+kWidth + 1 );
                mesh.appendTriangle( offset, offset+kWidth+1, offset+1 );
            }
            
            // add vertex
            float value = 80.0f * mPerlin.fBm(vec3(float(h), float(w), 0.f) * 0.005f);
            mVertices.push_back( vec3(float(w), value, float(h)) );
            //mesh.appendVertex( vec3(float(w), value, float(h)));
            // add texture coordinates
            // note: we only want to draw the lower part of the frequency bands,
            //  so we scale the coordinates a bit
            const float part = 0.5f;
            float s = w / float(kWidth-1);
            float t = h / float(kHeight-1);
           //mesh.appendTexCoord( vec2(part - part * s, t) );
            
            // add vertex colors
            //colors.push_back( h % 2 == 0 || true ? Color(CM_HSV, s, 1.0f, 1.0f) : Color(CM_RGB, s, s, s) );
            mesh.appendColorRgb(Color(CM_HSV, s, 1.0f, 1.0f));
        }
    }

    mShaderNum = 0;
    try {
                mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT0 ), loadResource( GLSL_FRAG0 ) ));
        //        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT1 ), loadResource( GLSL_FRAG2 ) ));
        //        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG1 ) ));
        //        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG2 ) ));
        //mShader.push_back(gl::GlslProg::create( loadResource( GLSL_BASIC_VERT ), loadResource( GLSL_BASIC_FRAG ) ));
    }
    catch( const std::exception& e ) {
        cout << e.what() << std::endl;
        return;
    }
    mBatch = gl::Batch::create(mesh, mShader[0]);
    std::vector<Colorf>     colors;
    std::vector<vec2>      coords;
    std::vector<uint32_t>	indices;
    
    for(size_t h=0;h<kHeight;++h)
    {
        for(size_t w=0;w<kWidth;++w)
        {
            // add polygon indices
            if(h < kHeight-1 && w < kWidth-1)
            {
                size_t offset = mVertices.size();

                indices.push_back(offset);
                indices.push_back(offset+kWidth);
                indices.push_back(offset+kWidth+1);
                indices.push_back(offset);
                indices.push_back(offset+kWidth+1);
                indices.push_back(offset+1);
            }
            
            // add vertex
            float value = 80.0f * mPerlin.fBm(vec3(float(h), float(w), 0.f) * 0.005f);
            mVertices.push_back( vec3(float(w), value, float(h)) );
            
            // add texture coordinates
            // note: we only want to draw the lower part of the frequency bands,
            //  so we scale the coordinates a bit
            const float part = 0.5f;
            float s = w / float(kWidth-1);
            float t = h / float(kHeight-1);
            coords.push_back( vec2(part - part * s, t) );
            
            // add vertex colors
            colors.push_back( h % 2 == 0 || true ? Color(CM_HSV, s, 1.0f, 1.0f) : Color(CM_RGB, s, s, s) );
        }
    }
    
//    gl::VboMesh::Layout layout;
//    layout.setStaticPositions();
////    layout.setDynamicPositions();
//    layout.setStaticColorsRGB();
//    layout.setStaticIndices();
//    layout.setStaticTexCoords2d();
//    
//    vboMeshRef = gl::VboMesh::create(100, 1, <#const std::vector<Layout> &vertexArrayLayouts#>)(mVertices.size(), indices.size(), layout, GL_TRIANGLES);
// 
//    mMesh.bufferPositions(mVertices);
//    mMesh.bufferColorsRGB(colors);
//    mMesh.bufferIndices(indices);
//    mMesh.bufferTexCoords2d(0, coords);
    
//    gl::VboMesh::VertexIter iter = mMesh.mapVertexBuffer();
//    for( int idx = 0; idx < mMesh.getNumVertices(); ++idx ) {
//        iter.setPosition(mVertices [idx]);
//        ++iter;
//    }

    vector<vec3> normals;
    
    // Iterate through again to set normals
//    for ( int32_t y = 0; y < mResolution.y - 1; y++ ) {
//        for ( int32_t x = 0; x < mResolution.x - 1; x++ ) {
//            vec3 vert0 = positions[ indices[ ( x + mResolution.x * y ) * 6 ] ];
//            vec3 vert1 = positions[ indices[ ( ( x + 1 ) + mResolution.x * y ) * 6 ] ];
//            vec3 vert2 = positions[ indices[ ( ( x + 1 ) + mResolution.x * ( y + 1 ) ) * 6 ] ];
//            normals[ x + mResolution.x * y ] = vec3( ( vert1 - vert0 ).cross( vert1 - vert2 ).normalized() );
//        }
//    }

    mIsMouseDown = false;
    mMouseUpDelay = 5.0;
    mMouseUpTime = getElapsedSeconds() - mMouseUpDelay;
    
    // the texture offset has two purposes:
    //  1) it tells us where to upload the next spectrum data
    //  2) we use it to offset the texture coordinates in the shader for the scrolling effect
    mOffset = 0;


    setFrameRate(30.0f);
}

void AudioVisualizerApp::shutdown()
{

}

void AudioVisualizerApp::update()
{
    if( mCapture && mCapture->checkNewFrame() ) {
        mTextureRef = gl::Texture::create( *mCapture->getSurface() );
    }
    
    mFrameRate = getAverageFps();

    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;

    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);

    // increment texture offset
    mOffset = (mOffset+1) % kHistory;

    // clear the spectrum for this row to avoid old data from showing up
    ////NOT SURE THIS IS NEEDED -- the texture will be overwritten completely next time by data
//    pDataLeft = mChannelLeft.getData() + kBands * mOffset;
//    pDataRight = mChannelRight.getData() + kBands * mOffset;
//    memset( pDataLeft, 0, kBands * sizeof(float) );
//    memset( pDataRight, 0, kBands * sizeof(float) );

    // animate camera if mouse has not been down for more than 30 seconds

    if(true || !mIsMouseDown && (getElapsedSeconds() - mMouseUpTime) > mMouseUpDelay)
    {
        
        float t = float( getElapsedSeconds() );
        float x = 0.5f * math<float>::cos( t * 0.07f );
        float y = 0.5f * math<float>::sin( t * 0.09f );//0.1f - 0.2f * math<float>::sin( t * 0.09f );
        float z = 0.05f * math<float>::sin( t * 0.05f ) - 0.15f;
       
        vec3 eye = vec3(kWidth * x, kHeight * y*0.1f, kHeight * z);

        x = 1.0f - x;
        y = -0.5f;
        z = 0.6f + 0.2f *  math<float>::sin( t * 0.12f );
        
        vec3 interest = vec3(kWidth * x, kHeight * y*0.1f, kHeight * z);
        //cout<<interest<< " "<< (eye.lerp(0.995f, mCamera.getEyePoint()))<<endl;
        
        // gradually move to eye position and center of interest
        if ( mVolumeMax < mMonitorSpectralNode->getVolume()){
            mVolumeMax = mMonitorSpectralNode->getVolume();
            cout<<"VOLUME "<< mVolumeMax << endl;
        }
    
        float correction = 1.0 - 0.1*mMonitorSpectralNode->getVolume();
//        mCamera.setEyePoint( eye.lerp(0.995f*correction, mCamera.getEyePoint()) );
//        mCamera.setCenterOfInterestPoint( interest.lerp(0.990f*correction, mCamera.getCenterOfInterestPoint()) );
        
        
//        if (mAutomaticSwitch &&  (mMonitorSpectralNode->getVolume() < 0.001f || mMonitorSpectralNode->getVolume() > 0.5f)){
//            mShaderNum = mShaderNum == mShader.size() - 1 ? 0 : mShaderNum + 1;
//        }
    }

    mPerlinMove++;


//    for(size_t h = 0 ; h < kHeight; ++h) {
//        for(size_t w = 0 ; w < kWidth; ++w) {
//            size_t i = h * kWidth + w;
//            if (w < kWidth - 1) {
//                mVertices [i].y = mVertices [i+1].y;
//            } else {
//                float value = 80.0f*mPerlin.fBm(vec3(float(h+ mPerlinMove), float(w), 0.f)* 0.005f);
//                mVertices[i].y = value;
//            }
//        }
//    }

//    mMesh.bufferPositions(mVertices);

//    gl::VboMesh::VertexIter iter = mMesh.mapVertexBuffer();
//    int w = 0;
//    int h = 0;
//    for( int idx = 0; idx < mMesh.getNumVertices(); ++idx ) {
//        if (w < kWidth -1) {
//            iter.setPosition(w,mVertices [idx+1].y, h);
//        } else {
//            float value = 80.0f*mPerlin.fBm(vec3(float(h+ mPerlinMove), float(w), 0.f)* 0.005f);
//            iter.setPosition( w,value, h);
//        }
//        ++iter;
//        ++w;
//        if ( w == kWidth){
//            w = 0;
//            h++;
//        }
//    }

}


void AudioVisualizerApp::draw()
{
    if (!mTextureRef) return;
    static float rotation = 0.0f;
    gl::clear();
    gl::setMatrices( mCamera );
    gl::pushMatrices();
	//gl::setMatricesWindow(getWindowSize());
    //gl::multViewMatrix(ci::rotate(rotation += 0.01, vec3(0,1,0)));
   // gl::multViewMatrix(ci::rotate(0.f, vec3(0,1,0)));

//    {
//    gl::ScopedModelMatrix scopeModel;
//    gl::multModelMatrix(ci::translate(vec3(1,0,0)));
//    mBatch->draw();
//    }
//    return;
    
    {
       /// gl::ScopedModelMatrix scopeModel;
        //gl::multModelMatrix(ci::translate(vec3(1,0,0)));
        float offSt = mOffset / float(kHistory);
        mBatch->getGlslProg()->uniform("uTexOffset", offSt);
        mBatch->getGlslProg()->uniform("resolution", 0.5f*(float)kWidth);
        mBatch->getGlslProg()->uniform("uVideoTex", 0);
        mBatch->getGlslProg()->uniform("uLeftTex", 1);
        mBatch->getGlslProg()->uniform("uRightTex", 2);

        
       // gl::ScopedModelMatrix scopeModel;
       // gl::multModelMatrix(ci::translate(vec3(1,0,0)));
        mShader[mShaderNum]->bind();
        mTextureRef->bind(0);
        mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
        mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
        mTextureLeft->bind(1);
        mTextureRight->bind(2);
        mBatch->draw();

        mTextureRight->unbind();
        mTextureLeft->unbind();
        mTextureRef->unbind();
    }
    gl::disableAlphaBlending();
    gl::popMatrices();
    mParams.draw();
    return;
    gl::enableDepthRead();
    gl::enableDepthWrite();
    // use camera
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    {

        // bind shader
        mShader[mShaderNum]->bind();
        float offSt = mOffset / float(kHistory);
        mShader[mShaderNum]->uniform("uTexOffset", offSt);
        mShader[mShaderNum]->uniform("uLeftTex", 0);
        mShader[mShaderNum]->uniform("uRightTex", 1);
        mShader[mShaderNum]->uniform("resolution", 0.5f*(float)kWidth);
        //mShader.uniform("elTime", (float) getElapsedFrames());
        
        // create textures from our channels and bind them
        mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
        mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
        
//        mTextureLeft->enableAndBind();
        mTextureRight->bind(1);
        
        // draw mesh using additive blending
        gl::enableAdditiveBlending();

        gl::color( Color(1, 1, 1) );
//        gl::draw( mMesh );

        gl::disableAlphaBlending();

        // unbind textures and shader
        mTextureRight->unbind();
        mTextureLeft->unbind();
//        mShader[mShaderNum]->unbind();
    }

    gl::popMatrices();
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    mParams.draw();
    
}

void AudioVisualizerApp::mouseDown( MouseEvent event )
{
    // handle mouse down
    mIsMouseDown = true;
    
//    mMayaCam.setCurrentCam(mCamera);
//    mMayaCam.mouseDown( event.getPos() );
    //cout<<mMayaCam.getCamera().getEyePoint()<<endl;
}

void AudioVisualizerApp::mouseDrag( MouseEvent event )
{
    // handle mouse drag
//    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
//    mCamera = mMayaCam.getCamera();
    //cout<<"EYEPOINT "<<mMayaCam.getCamera().getEyePoint()<<endl;
    //cout<<"COI "<<mMayaCam.getCamera().getCenterOfInterestPoint()<<endl;
}

void AudioVisualizerApp::mouseUp( MouseEvent event )
{
    // handle mouse up
    //mMouseUpTime = getElapsedSeconds();
    mIsMouseDown = false;
}

void AudioVisualizerApp::keyDown( KeyEvent event )
{
    // handle key down
    switch( event.getCode() )
    {
        case KeyEvent::KEY_ESCAPE:
            break;
        case KeyEvent::KEY_F4:
            if( event.isAltDown() )
            break;
        case KeyEvent::KEY_LEFT:
            
            break;
        case KeyEvent::KEY_RIGHT:
            break;
        case KeyEvent::KEY_f:
            //setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_o:
            break;
        case KeyEvent::KEY_p:
            break;
        case KeyEvent::KEY_s:
            
            break;
    }
}

void AudioVisualizerApp::resize()
{
    // handle resize
    mCamera.setAspectRatio( getWindowAspectRatio() );
}

CINDER_APP( AudioVisualizerApp, RendererGl )