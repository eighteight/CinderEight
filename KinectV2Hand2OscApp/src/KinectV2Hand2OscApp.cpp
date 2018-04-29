#include "cinder/app/App.h"
#include "cinder/params/Params.h"

#define NOMINMAX

#include "Osc.h"

#include "Kinect2.h"

#include "cinder/gl/gl.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"

#include "cinder/audio/Context.h"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/Utilities.h"
#include "cinder/Log.h"

const float MAX_VOLUME = 0.6f;
const size_t MAX_SPLASHES = 200;
const float MAX_RADIUS = 300;
const float MAX_PITCH_MIDI = 80;
const float	MIN_PITCH_MIDI = 40;

using namespace ci;
using namespace ci::app;
using namespace std;

#define USE_UDP 1

#if USE_UDP
using Sender = osc::SenderUdp;
#else
using Sender = osc::SenderTcp;
#endif

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 10001;
const uint16_t localPort = 10000;

class KinectV2HandToOsc : public ci::app::App
{
public:
	KinectV2HandToOsc();
	void						draw() override;
	void						setup() override;
	void						update() override;
	void						keyDown(KeyEvent event) override;
private:
	Kinect2::BodyFrame			mBodyFrame;
	ci::Channel8uRef			mChannelBodyIndex;
	ci::Channel16uRef			mChannelDepth;
	Kinect2::DeviceRef			mDevice;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;


	// This will maintain a list of points which we will draw line segments between

	std::vector<vector<glm::vec2>> mLines;
	void clearLines();


	int		findNearestPt(const vec2 &aPt);
	int		mTrackedPoint;
	float   dist(vec2 p, vec2 f);
	float   mMinDistance = 2.0f;
	size_t  mLineIndex = 0;
	bool    mStartLine = false;
	float   mLineStopSeconds;
	int     mLineWidth = 20.0f;

	//audio
	void	setVariableDelayMod();
	float   quantizePitch(const vec2 &pos);
	audio::GenOscNodeRef	mOsc;
	audio::DelayNodeRef		mDelay;
	audio::GainNodeRef		mGain;

	Sender	mSender;
	bool	mIsConnected;
	void onSendError(asio::error_code error);
};

KinectV2HandToOsc::KinectV2HandToOsc()
	: mSender(localPort, destinationHost, destinationPort), mIsConnected(false)
{
}

void KinectV2HandToOsc::draw()
{
	const gl::ScopedViewport scopedViewport(ivec2(0), getWindowSize());
	const gl::ScopedMatrices scopedMatrices;
	const gl::ScopedBlendAlpha scopedBlendAlpha;
	gl::setMatricesWindow(getWindowSize());
	gl::clear();
	gl::color(ColorAf::white());
	gl::disableDepthRead();
	gl::disableDepthWrite();

	if (mChannelBodyIndex) {
		//		gl::enable( GL_TEXTURE_2D );

		gl::pushMatrices();
		//console() << (vec2(getWindowSize()) / vec2(mChannelBodyIndex->getSize())) << endl;
		//gl::scale( vec2( getWindowSize() ) / vec2( mChannelBodyIndex->getSize() ) );
		//gl::disable( GL_TEXTURE_2D );

		try {
			for (const Kinect2::Body& body : mBodyFrame.getBodies()) {
				if (body.isTracked()) {
					float z = body.getJointMap().at(JointType_HandRight).getPosition().z;
					if (z < 1.0f) {
						if (!mStartLine) { mStartLine = true; }
						vec3 handPosition = body.getJointMap().at(JointType_HandRight).getPosition();
						vec2 clickPt = mDevice->mapCameraToDepth(handPosition);
						int nearestIdx = findNearestPt(clickPt);
						auto pnts = mLines[mLineIndex];
						if ((nearestIdx < 0) || (dist(pnts[nearestIdx], clickPt) > mMinDistance)) {
							mLines[mLineIndex].push_back(clickPt);
							mTrackedPoint = -1;

							if (!mIsConnected)
								return;

							osc::Message msg("/mousemove/1");
							msg.append((int) clickPt.x);
							msg.append((int) clickPt.y);
							msg.append((int)(handPosition.z*10));
							cout << clickPt.x << endl;
							// Send the msg and also provide an error handler. If the message is important you
							// could store it in the error callback to dispatch it again if there was a problem.
							mSender.send(msg, std::bind(&KinectV2HandToOsc::onSendError, this, std::placeholders::_1));
						}

						float freq = quantizePitch(clickPt);
						float gain = 1.0f - (float)clickPt.y / (float)getWindowHeight();
						gain *= MAX_VOLUME;
						mOsc->getParamFreq()->applyRamp(freq, 0.04f);
						mGain->getParam()->applyRamp(gain, 0.1f);
						mLineStopSeconds = getElapsedSeconds();
					}
					else {
						if (mStartLine) {
							mLineIndex++;
							mLines.push_back({});
							mStartLine = false;
						}
					}

					gl::clear(Color::white());
					gl::color(0.0f, 0.0f, 0.0f);

					gl::lineWidth((int)mLineWidth);
					for (const auto &line : mLines) {
						gl::begin(GL_LINE_STRIP);
						for (const vec2 &point : line) {
							vec2 point2 = vec2(point.x*2.2, point.y);
							gl::vertex(point2*2.0f);
						}
						gl::end();
					}
					gl::popMatrices();
				}
			}
		}
		catch (...) {
			console() << "oops" << endl;
		}
	}

	mParams->draw();
}

void KinectV2HandToOsc::setVariableDelayMod()
{
	mDelay->setMaxDelaySeconds(4);

	auto ctx = audio::master();

	auto osc = ctx->makeNode(new audio::GenSineNode(0.00113f, audio::Node::Format().autoEnable()));
	auto mul = ctx->makeNode(new audio::GainNode(0.3f));
	auto add = ctx->makeNode(new audio::AddNode(0.343f));

	osc >> mul >> add;
	mDelay->getParamDelaySeconds()->setProcessor(add);
}

void KinectV2HandToOsc::clearLines()
{
	mLines.clear();
	mLines = { std::vector<glm::vec2>() };
	mLineIndex = 0;
	mGain->getParam()->applyRamp(0, 1.1f);

	osc::Message msg("/stop/1");
	// Send the msg and also provide an error handler. If the message is important you
	// could store it in the error callback to dispatch it again if there was a problem.
	mSender.send(msg, std::bind(&KinectV2HandToOsc::onSendError, this, std::placeholders::_1));
}

// returns a quantized pitch (in hertz) within the lydian dominant scale
float KinectV2HandToOsc::quantizePitch(const vec2 &pos)
{
	const size_t scaleLength = 7;
	float scale[scaleLength] = { 0, 2, 4, 6, 7, 9, 10 };

	int pitchMidi = lroundf(lmap(pos.x, 0.0f, (float)getWindowWidth(), MIN_PITCH_MIDI, MAX_PITCH_MIDI));

	bool quantized = false;
	while (!quantized) {
		int note = pitchMidi % 12;
		for (size_t i = 0; i < scaleLength; i++) {
			if (note == scale[i]) {
				quantized = true;
				break;
			}
		}
		if (!quantized)
			pitchMidi--;
	}

	return audio::midiToFreq(pitchMidi);
}

void KinectV2HandToOsc::keyDown(KeyEvent event)
{
	if (event.getChar() == 'f') {
		// Toggle full screen when the user presses the 'f' key.
		setFullScreen(!isFullScreen());
	}
	else if (event.getCode() == KeyEvent::KEY_SPACE) {
		// Clear the list of points when the user presses the space bar.
		clearLines();
	}
	else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		// Exit full screen, or quit the application, when the user presses the ESC key.
		if (isFullScreen())
			setFullScreen(false);
		else
			quit();
	}
}

int KinectV2HandToOsc::findNearestPt(const vec2 &aPt)
{
	auto pnts = mLines[mLineIndex];
	if (pnts.empty())
		return -1;

	int result = 0;
	float nearestDist = dist(pnts[0], aPt);
	for (size_t i = 1; i < pnts.size(); ++i) {
		if (dist(pnts[i], aPt) < nearestDist) {
			result = i;
			nearestDist = dist(pnts[i], aPt);
		}
	}
	return result;
}

float KinectV2HandToOsc::dist(vec2 p, vec2 f)
{
	vec2 delta = p - f;
	return length(delta);
}

void KinectV2HandToOsc::setup()
{
	mFrameRate = 0.0f;
	mFullScreen = false;

	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyEventHandler([&](const Kinect2::BodyFrame frame)
	{
		mBodyFrame = frame;
	});
	mDevice->connectBodyIndexEventHandler([&](const Kinect2::BodyIndexFrame frame)
	{
		mChannelBodyIndex = frame.getChannel();
	});
	mDevice->connectDepthEventHandler([&](const Kinect2::DepthFrame frame)
	{
		mChannelDepth = frame.getChannel();
	});

	mParams = params::InterfaceGl::create("Written in 4 hours", ivec2(200, 100));
	mParams->addParam("Min Dist", &mMinDistance).min(0.1f).max(100.5f).keyIncr("m").keyDecr("M").precision(2).step(1.0f);
	mParams->addParam("Line width", &mLineWidth).min(5.0f).max(100.5f).keyIncr("l").keyDecr("L").precision(2).step(2.0f);;
	mParams->addParam("Frame rate", &mFrameRate, "", true);
	mParams->addParam("Full screen", &mFullScreen).key("f");
	mParams->addButton("Quit", [&]() { quit(); }, "key=q");

	auto ctx = audio::master();
	mOsc = ctx->makeNode(new audio::GenOscNode);
	mGain = ctx->makeNode(new audio::GainNode(0));
	mDelay = ctx->makeNode(new audio::DelayNode);

	mOsc->setWaveform(audio::WaveformType::TRIANGLE);

	// The Delay's length Param is itself controlled with Node's, which is configured next.
	setVariableDelayMod();

	// Now we connect up the Node's so that the signal immediately reaches the Context's OutputNode, but it also
	// feedback in a cycle to create an echo. To control the level of feedback and prevent ringing, a one-off GainNode
	// is used with a value of 0.5, which gives a fairly natural sounding decay.

	auto feedbackGain = audio::master()->makeNode(new audio::GainNode(0.5f));
	feedbackGain->setName("FeedbackGain");

	mOsc >> mGain >> ctx->getOutput();
	mGain >> mDelay >> feedbackGain >> mDelay >> ctx->getOutput();

	mOsc->enable();
	ctx->enable();

	mLines = { std::vector<glm::vec2>() };

	try {
		// Bind the sender to the endpoint. This function may throw. The exception will
		// contain asio::error_code information.
		mSender.bind();
		mIsConnected = true;
	}
	catch (const osc::Exception &ex) {
		CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
		quit();
	}
}

void KinectV2HandToOsc::update()
{
	mFrameRate = getAverageFps();

	if (mFullScreen != isFullScreen()) {
		setFullScreen(mFullScreen);
		mFullScreen = isFullScreen();
	}

	if (getElapsedSeconds() - mLineStopSeconds > 5) {
		clearLines();
	}
}

// Unified error handler. Easiest to have a bound function in this situation,
// since we're sending from many different places.
void KinectV2HandToOsc::onSendError(asio::error_code error)
{
	if (error) {
		CI_LOG_E("Error sending: " << error.message() << " val: " << error.value());
		// If you determine that this error is fatal, make sure to flip mIsConnected. It's
		// possible that the error isn't fatal.
		mIsConnected = false;
		try {
#if ! USE_UDP
			// If this is Tcp, it's recommended that you shutdown before closing. This
			// function could throw. The exception will contain asio::error_code
			// information.
			mSender.shutdown();
#endif
			// Close the socket on exit. This function could throw. The exception will
			// contain asio::error_code information.
			mSender.close();
		}
		catch (const osc::Exception &ex) {
			CI_LOG_EXCEPTION("Cleaning up socket: val -" << ex.value(), ex);
		}
		quit();
	}
}


CINDER_APP(KinectV2HandToOsc, RendererGl, [](App::Settings* settings)
{
	settings->prepareWindow(Window::Format().size(1024, 768).title("SLOVOFORMA PROTOTYPE"));
	settings->setFrameRate(60.0f);
})
