#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

/**
 InfiniteTerrainApp:
 
 Demonstrates shader loading and uniform passing.
 Modeled after shadertoy.com, which is a more fully-fledged
 solution to experimenting with fragment shaders.
 Shaders that don't use channels should work unmodified, except for
 changing gl_FragColor to oColor and adding the uniform declaration.
 
 Loads default.fs by default.
 Try dragging ripple.fs or your own fragment shader onto the app window.
 Press any key to reload the last shader from disk.
 */

class InfiniteTerrainApp : public AppNative {
public:
    void prepareSettings( Settings *settings );
    void setup();
    void update();
    void draw();
    void loadShader( const fs::path &fragment_path );
    void resize();
private:
    gl::GlslProgRef	mProg, mProgLines;
    gl::VboMeshRef	mMesh;
    ci::vec4		mMouseCoord;
    fs::path		mCurrentShaderPath;
    
    bool			mDoSave = false;
    
    float mFrameRate;
    ci::params::InterfaceGl		mParams;
};

void InfiniteTerrainApp::resize(){

}

void InfiniteTerrainApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 800, 420 ); // 16:9 display
}

void InfiniteTerrainApp::setup()
{
    mFrameRate			= 0.0f;
    mParams = params::InterfaceGl( "InfiniteTerrain", vec2( 200, 100 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,									"", true									);

    
    // load fragment shader from the provided path
    // we always use the same vertex shader, so it isn't specified
    mProgLines = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("spectrum.vert")).fragment(loadAsset("default.fs")));
    loadShader( getAssetPath( "moon-surface.fs" ) );
    // create a rectangle to be drawn with our shader program
    // default is from -0.5 to 0.5, so we scale by 2 to get -1.0 to 1.0
    mMesh = gl::VboMesh::create( geom::Rect(Rectf (-1.f, -1.f, 1.0f, 1.0f)));
    geom::Plane();
    
    // load a new shader on file drop
    getWindow()->getSignalFileDrop().connect( [this]( FileDropEvent &event )
                                             {
                                                 auto file = event.getFile( 0 );
                                                 if( fs::is_regular_file( file ) );
                                                 { loadShader( file ); }
                                             } );
    // set iMouse XY on mouse move
    getWindow()->getSignalMouseMove().connect( [this]( MouseEvent &event )
                                              {
                                                  mMouseCoord.x = event.getX();
                                                  mMouseCoord.y = getWindowHeight() - event.getY();
                                              } );
    // set iMouse ZW on mouse down
    getWindow()->getSignalMouseDown().connect( [this]( MouseEvent &event )
                                              {
                                                  mMouseCoord.z = event.getX();
                                                  mMouseCoord.w = getWindowHeight() - event.getY();
                                              } );
    // reload last fragment shader on keypress, or save frame if key == 's'
    getWindow()->getSignalKeyDown().connect( [this]( KeyEvent &event )
                                            {
                                                if( event.getCode() == KeyEvent::KEY_s )
                                                { mDoSave = true; }
                                                else
                                                { loadShader( mCurrentShaderPath ); }
                                            } );
    // set shader resolution uniform on resize (if shader has that uniform)
    getWindow()->getSignalResize().connect( [this]()
                                           {
                                               auto map = mProg->getActiveUniformTypes();
                                               if( map.find( "iResolution" ) != map.end() )
                                               { mProg->uniform( "iResolution", vec3( getWindowWidth(), getWindowHeight(), 0.0f ) ); }
                                           } );
}

void InfiniteTerrainApp::loadShader( const fs::path &fragment_path )
{
    try
    {	// load and compile our shader
        mProg = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "default.vs" ) )
                                     .fragment( loadFile( fragment_path ) ) );

        // no exceptions occurred, so store the shader's path for reloading on keypress
        mCurrentShaderPath = fragment_path;
        // check that uniforms exist before setting the constant uniforms
        auto map = mProg->getActiveUniformTypes();
        console() << "Successfully compiled fragment shader from: " << mCurrentShaderPath << endl;
        console() << "Found uniforms:" << endl;
        for( const auto &pair : map )
        {
            console() << pair.first << endl;
        }
        console() << endl;
        if( map.find( "iResolution" ) != map.end() )
        { mProg->uniform( "iResolution", vec3( getWindowWidth(), getWindowHeight(), 0.0f ) ); }
    }
    catch( ci::gl::GlslProgCompileExc &exc )
    {
        console() << "Error compiling shader: " << exc.what() << endl;
    }
    catch ( ci::Exception &exc )
    {
        console() << "Error loading shader: " << exc.what() << endl;
    }
}

void InfiniteTerrainApp::update()
{
    mFrameRate = getAverageFps();
    // get the current time with second-level accuracy
    auto now = boost::posix_time::second_clock::local_time();
    auto date = now.date();
    auto time = now.time_of_day();
    // set each uniform if it exists in the shader program
    // when compiled, only uniforms that are used remain in the program
    auto map = mProg->getActiveUniformTypes();
    if( map.find( "iGlobalTime" ) != map.end() )
    { mProg->uniform( "iGlobalTime", static_cast<float>( getElapsedSeconds() ) ); }
    if( map.find( "iDate" ) != map.end() )
    { mProg->uniform( "iDate", vec4( date.year(), date.month(), date.day_number(), time.total_seconds() ) ); }
    if( map.find( "iMouse" ) != map.end() )
    { mProg->uniform( "iMouse", mMouseCoord ); }
    if( map.find( "iResolution" ) != map.end() )
    { mProg->uniform( "iResolution", vec3( getWindowWidth(), getWindowHeight(), 0.0f ) );}
}

void InfiniteTerrainApp::draw()
{
    gl::viewport(0, 0, getWindowBounds().x2, getWindowBounds().y2 );
    // clear out the window with black
    gl::clear( Color( 0, 0, 0 ) );
    // use our shader for this draw loop
    gl::ScopedGlslProg prog(mProg);
    
    gl::ScopedGlslProg progLines(mProgLines);

    // draw our screen rectangle
    gl::draw( mMesh );
    
    // if you want to save a screenshot
    if( mDoSave )
    {
        mDoSave = false;
        try
        { writeImage( getSaveFilePath(), copyWindowSurface() ); }
        catch( ci::Exception &exc )
        { console() << "Failed to save image: " << exc.what() << endl; }
    }
    
    mParams.draw();
}

CINDER_APP_NATIVE( InfiniteTerrainApp, RendererGl )
