#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "UnionJack.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App {
public:
    void setup() override;
    void draw() override;
    void mouseWheel(MouseEvent event) override;
    
    vector<UnionJack>   mDisplays;
    float               mRotor = 0.0f;
};

void CinderProjectApp::setup()
{
    Color light = Color8u::hex( 0x42a1eb );
    Color dark = Color8u::hex( 0x082f4d );
    vec2 padding( 20 );
    
    mDisplays = {
        UnionJack( "UnionJack *" ).scale( 3 ).position( padding )
        .colors( Color8u::hex( 0xf00000 ), Color8u::hex( 0x530000 ) ),
        // Let's print out the full ASCII table as a font specimen
        UnionJack( 33 ).display( " !\"#$%&'()*+,-./0123456789:;<=>?"   ).colors( light, dark ),
        UnionJack( 33 ).display( "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"   ).colors( light, dark ),
        UnionJack( 33 ).display( "`abcdefghijklmnopqrstuvwxyz{|}~\x7F" ).colors( light, dark ),
    };
    // Position the displays relative to each other.
    mDisplays[1].below( mDisplays[0] );
    mDisplays[2].below( mDisplays[1] );
    mDisplays[3].below( mDisplays[2] );
    
    setWindowSize( padding + mDisplays[3].calcBoundingBox().getLowerRight() );
}

void CinderProjectApp::draw()
{
    gl::clear( Color::black() );
    for ( auto display = mDisplays.begin(); display != mDisplays.end(); ++display ) {
        display->draw();
    }
}

void CinderProjectApp::mouseWheel(MouseEvent event)
{
    // Scroll the mouse and animate the display.
    mRotor += event.getWheelIncrement();
    char pattern[8] = { 0xA, 0x8, 0xF, 0xE, 0xD, 0x9, 0xC, 0xB };
    for (int i = 0; i < 22; i++) {
        int pos = (int) mRotor % 8;
        uint16_t val = 1 << ( mRotor > 0 ? pattern[pos] : abs( pos ) );
        val = rand() * 8;
        mDisplays[0].display( i, val );
        
    }
}

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
}

CINDER_APP( CinderProjectApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )