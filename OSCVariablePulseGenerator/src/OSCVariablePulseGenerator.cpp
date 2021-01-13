#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Log.h"
#include "cinder/Timeline.h"
#include "cinder/osc/Osc.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using Receiver = osc::ReceiverUdp;
using protocol = asio::ip::udp;
using Sender = osc::SenderUdp;


const uint16_t localPort = 10002;

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 1234;

class OSCVariablePulseGenerator : public App {
  public:
	OSCVariablePulseGenerator();
	void setup() override;
	void draw() override;
    void update() override;

    void send(int num);
    void onSendError( asio::error_code error );
    
    uint    mFramesToSkip;
    float   mLastFrame;
    bool    mSendSignal;
	
	Receiver mReceiver;
	std::map<uint64_t, protocol::endpoint> mConnections;
    
    Sender    mSender;
    bool    mIsConnected;
};

OSCVariablePulseGenerator::OSCVariablePulseGenerator()
: mReceiver( localPort ), mSender( localPort, destinationHost, destinationPort ), mIsConnected( false )
{
}

void OSCVariablePulseGenerator::send( int num)
{
    // Make sure you're connected before trying to send.
    if( ! mIsConnected )
        return;
    
    osc::Message msg( "/isadora/1" );
    msg.append( "123, i" );
    // Send the msg and also provide an error handler. If the message is important you
    // could store it in the error callback to dispatch it again if there was a problem.
    mSender.send( msg, std::bind( &OSCVariablePulseGenerator::onSendError,
                                 this, std::placeholders::_1 ) );
}

// Unified error handler. Easiest to have a bound function in this situation,
// since we're sending from many different places.
void OSCVariablePulseGenerator::onSendError( asio::error_code error )
{
//    if( error ) {
//        CI_LOG_E( "Error sending: " << error.message() << " val: " << error.value() );
//        // If you determine that this error is fatal, make sure to flip mIsConnected. It's
//        // possible that the error isn't fatal.
//        mIsConnected = false;
//        try {
//            // Close the socket on exit. This function could throw. The exception will
//            // contain asio::error_code information.
//            mSender.close();
//        }
//        catch( const osc::Exception &ex ) {
//            CI_LOG_EXCEPTION( "Cleaning up socket: val -" << ex.value(), ex );
//        }
//        quit();
//    }
}

void OSCVariablePulseGenerator::update()
{
    
    //mLastFrame
    auto elFrames = getElapsedFrames() - mLastFrame;
    if (elFrames == mFramesToSkip) {
        send (1);
    } else {
        send (0);
    }
}

void OSCVariablePulseGenerator::setup()
{
	mReceiver.setListener( "/variablepulsegenerator/1",
	[&]( const osc::Message &msg ){
        if (msg.getNumArgs() < 1) {
            return;
        }
        cinder::osc::ArgType tp = msg[0].getType();
        uint skip = -1;
        switch (tp) {
            case cinder::osc::ArgType::FLOAT:
                skip = (uint) msg[0].flt();
                break;
            case cinder::osc::ArgType::INTEGER_32:
                skip = (uint) msg[0].int32();
                break;
            case cinder::osc::ArgType::INTEGER_64:
                skip = (uint) msg[0].int64();
                break;
            case cinder::osc::ArgType::DOUBLE:
                skip = (uint) msg[0].dbl();
                break;
             default:
                break;
        }
        if (skip == -1) {
            return;
        }
        mFramesToSkip = skip;
        cout << "SKIPS: " << mFramesToSkip << endl;
        //send(mCurrentNumber);
	});

	try {
		// Bind the receiver to the endpoint. This function may throw.
		mReceiver.bind();
	}
	catch( const osc::Exception &ex ) {
		CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
		quit();
	}

	// UDP opens the socket and "listens" accepting any message from any endpoint. The listen
	// function takes an error handler for the underlying socket. Any errors that would
	// call this function are because of problems with the socket or with the remote message.
	mReceiver.listen(
	[]( asio::error_code error, protocol::endpoint endpoint ) -> bool {
		if( error ) {
			CI_LOG_E( "Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint );
			return false;
		}
		else
			return true;
	});
    mIsConnected = true;
}

void OSCVariablePulseGenerator::draw()
{
    // Clear the contents of the window. This call will clear
    // both the color and depth buffers.
    gl::clear( Color::gray( 0.1f ) );
    
    // Set the current draw color to orange by setting values for
    // red, green and blue directly. Values range from 0 to 1.
    // See also: gl::ScopedColor
    gl::color( 1.0f, 0.5f, 0.25f );
    gl::drawSolidCircle(getWindowCenter(), 100.0f);
    
    // We're going to draw a line through all the points in the list
    // using a few convenience functions: 'begin' will tell OpenGL to
    // start constructing a line strip, 'vertex' will add a point to the
    // line strip and 'end' will execute the draw calls on the GPU.

}

auto settingsFunc = []( App::Settings *settings ) {
#if defined( CINDER_MSW )
	settings->setConsoleWindowEnabled();
#endif
	settings->setMultiTouchEnabled( false );
};

CINDER_APP( OSCVariablePulseGenerator, RendererGl, settingsFunc )
