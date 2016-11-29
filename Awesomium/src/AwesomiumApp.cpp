#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderAwesomium.h"
#include "DialogWebViewListener.h"
#include "WebPreferences.h"
#if defined( CINDER_COCOA )
#include "cinder/app/cocoa/PlatformCocoa.h"
#endif
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Awesomium;

class AwesomiumApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseUp(MouseEvent) override;
    void mouseMove (MouseEvent) override;
    void mouseDrag (MouseEvent) override;
	void update() override;
	void draw() override;
    void quit() override;
    void resize() override;
    void keyDown( KeyEvent event ) override;
    void keyUp( KeyEvent event ) override;
private:
	Awesomium::WebCore*		mWebCorePtr;
	Awesomium::WebView*		mWebViewPtr;
	
	gl::TextureRef				mWebTexture;
	gl::TextureRef				mLoadingTexture;
    
	Font					mFont;
    Awesomium::NativeWindow wind;
};

void AwesomiumApp::setup()
{
	// set Awesomium logging to verbose
	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;
#if defined( CINDER_MAC )
	std::string frameworkPath = ( getAppPath() / "Awesomium.app" / "Contents" / "MacOS" ).string();
    auto frameworkPath1 = PlatformCocoa::get()->getBundle();
	cnf.package_path = Awesomium::WebString::CreateFromUTF8( frameworkPath.c_str(), frameworkPath.size() );
#endif

	// initialize the Awesomium web engine
	mWebCorePtr = Awesomium::WebCore::Initialize( cnf );
    
    Awesomium::WebPreferences prefs;
    prefs.enable_gpu_acceleration = true;
    prefs.enable_web_gl = true;
    WebString my_string = WebString::CreateFromUTF8("", strlen(""));
    Awesomium::WebSession* my_session = mWebCorePtr->CreateWebSession(my_string, prefs);
    
	// create a webview
	mWebViewPtr = mWebCorePtr->CreateWebView( getWindowWidth(), getWindowHeight(), my_session );
    wind = (Awesomium::NativeWindow)getWindow()->getNative();
    mWebViewPtr->set_parent_window(wind);
	mWebViewPtr->LoadURL( Awesomium::WebURL( Awesomium::WSLit( "http://ro.me" ) ) );
	mWebViewPtr->Focus();
    
	// load and create a "loading" icon
	try { mLoadingTexture = gl::Texture::create( loadImage( loadAsset( "loading.png" ) ) ); }
	catch( const std::exception &e ) { console() << "Error loading asset: " << e.what() << std::endl; }
    
//    DialogWebViewListener *listener = new DialogWebViewListener();
//    mWebViewPtr->set_dialog_listener(listener);

}

void AwesomiumApp::mouseMove(cinder::app::MouseEvent event){
    	ph::awesomium::handleMouseMove( mWebViewPtr, event );
}

void AwesomiumApp::mouseUp(cinder::app::MouseEvent event){
    // send mouse events to Awesomium
	ph::awesomium::handleMouseUp( mWebViewPtr, event );
}

void AwesomiumApp::quit(){
    // properly shutdown Awesomium on exit
	if( mWebViewPtr ) mWebViewPtr->Destroy();
	Awesomium::WebCore::Shutdown();
}

void AwesomiumApp::resize(){
    // resize webview if window resizes
    if( mWebViewPtr )
        mWebViewPtr->Resize( getWindowWidth(), getWindowHeight() );
}

void AwesomiumApp::mouseDown( MouseEvent event )
{
    	ph::awesomium::handleMouseDown( mWebViewPtr, event );
}

void AwesomiumApp::mouseDrag(cinder::app::MouseEvent event){
    // send mouse events to Awesomium
	ph::awesomium::handleMouseDrag( mWebViewPtr, event );
}

void AwesomiumApp::update()
{
    // update the Awesomium engine
	mWebCorePtr->Update();
    
	// create or update our OpenGL Texture from the webview
	if( ph::awesomium::isDirty( mWebViewPtr ) )
	{
		try {
			// set texture filter to NEAREST if you don't intend to transform (scale, rotate) it
			gl::Texture::Format fmt;
			fmt.setMagFilter( GL_NEAREST );
            
			// get the texture using a handy conversion function
            //fmt.loadTopDown();
			mWebTexture = ph::awesomium::toTexture( mWebViewPtr, fmt );
            mWebTexture-> setTopDown( true );
		}
		catch( const std::exception &e ) {
			console() << e.what() << std::endl;
		}
        
		// update the window title to reflect the loaded content
		char title[1024];
		mWebViewPtr->title().ToUTF8( title, 1024 );
        
		app::getWindow()->setTitle( title );
	}

}

void AwesomiumApp::draw()
{
    
    //getWindow()->emitDraw();
	gl::clear();
    
	if( mWebTexture )
	{
		gl::color( Color::white() );
        gl::pushModelView();
        //gl::rotate(180.0f);
		gl::draw( mWebTexture );
        gl::popModelView();
	}
    
	// show spinner while loading
	if( mLoadingTexture && mWebViewPtr && mWebViewPtr->IsLoading() )
	{
		gl::pushModelView();
        
		gl::translate( 0.5f * vec2( getWindowSize() ) );
		gl::scale( 0.5f, 0.5f );
		gl::rotate( 180.0f * float( getElapsedSeconds() ) );
		gl::translate( -0.5f * vec2(mLoadingTexture->getSize()) );
		
		gl::color( Color::white() );
		gl::enableAlphaBlending();
		gl::draw( mLoadingTexture );
		gl::disableAlphaBlending();
        
		gl::popModelView();
	}

}

void AwesomiumApp::keyDown( KeyEvent event )
{
    // send key events to Awesomium
    ph::awesomium::handleKeyDown( mWebViewPtr, event );
    if (event.getChar() == 'b'){
        //getWindow().setBorderless(!getWindow().isBorderless());
    }
}

void AwesomiumApp::keyUp( KeyEvent event )
{
    // send key events to Awesomium
    ph::awesomium::handleKeyUp( mWebViewPtr, event );
}

CINDER_APP( AwesomiumApp, RendererGl )
