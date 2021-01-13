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

class OSCNumberReceiverSender : public App {
  public:
	OSCNumberReceiverSender();
	void setup() override;
	void draw() override;
    void render();
    void send(int num);
    void onSendError( asio::error_code error );
    int mCurrentNumber;
	
	ivec2	mCurrentCirclePos;
	vec2	mCurrentSquarePos;
	bool	mMouseDown = false;
	
	Receiver mReceiver;
	std::map<uint64_t, protocol::endpoint> mConnections;
    
    Sender    mSender;
    bool    mIsConnected;
    
    gl::TextureRef        mTextTexture;
    vec2                mSize;
    Font                mFont;
};

OSCNumberReceiverSender::OSCNumberReceiverSender()
: mReceiver( localPort ), mSender( localPort, destinationHost, destinationPort ), mIsConnected( false )
{
}

void OSCNumberReceiverSender::send( int num)
{
    // Make sure you're connected before trying to send.
    if( ! mIsConnected )
        return;
    
    osc::Message msg( "/isadora/1" );
    msg.append( num );
    // Send the msg and also provide an error handler. If the message is important you
    // could store it in the error callback to dispatch it again if there was a problem.
    mSender.send( msg, std::bind( &OSCNumberReceiverSender::onSendError,
                                 this, std::placeholders::_1 ) );
}

// Unified error handler. Easiest to have a bound function in this situation,
// since we're sending from many different places.
void OSCNumberReceiverSender::onSendError( asio::error_code error )
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


void OSCNumberReceiverSender::setup()
{
	mReceiver.setListener( "/oscappcontrol/1",
	[&]( const osc::Message &msg ){
        if (msg.getNumArgs() < 2) {
            return;
        }
        string command = msg[0].string();
        string exePath = msg[1].string();
        if (exePath.length() > 0 && command.length() > 0) {
            string fullCommand = command + " " + exePath;
            int res = system(fullCommand.c_str());
            cout<<res<<endl;
        }
        render();
        send(mCurrentNumber);
	});
    
    mReceiver.setListener( "/oscappcontrol/0",
                          [&]( const osc::Message &msg ){
                              string exePath = msg[0].string();
                              if (exePath.length() > 0 ) {
                                  string command = "open " + exePath;
                                  int res = system(command.c_str());
                                  cout<<res<<endl;
                              }
                              render();
                              send(mCurrentNumber);
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
    
    mFont = Font( "Times New Roman", 32 );
    
    mIsConnected = true;

}

void OSCNumberReceiverSender::render()
{
    string txt = to_string((int)(mCurrentNumber));
    TextBox tbox = TextBox().alignment( TextBox::RIGHT ).font( mFont ).size( ivec2( mSize.x, TextBox::GROW ) ).text( txt );
    tbox.setColor( Color( 1.0f, 0.65f, 0.35f ) );
    tbox.setBackgroundColor( ColorA( 0.5, 0, 0, 1 ) );
    mTextTexture = gl::Texture2d::create( tbox.render() );
    
    getWindow()->setTitle( txt);
}


void OSCNumberReceiverSender::draw()
{
    gl::setMatricesWindow( getWindowSize() );
    gl::enableAlphaBlending();
    gl::clear( Color( 0, 0, 0 ) );

    if( mTextTexture )
        gl::draw( mTextTexture );
}

auto settingsFunc = []( App::Settings *settings ) {
#if defined( CINDER_MSW )
	settings->setConsoleWindowEnabled();
#endif
	settings->setMultiTouchEnabled( false );
};

CINDER_APP( OSCNumberReceiverSender, RendererGl, settingsFunc )
