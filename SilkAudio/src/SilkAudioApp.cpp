/*
 *  IKLine.cpp
 *  Paint
 *
 *  Created by Mario.Gonzalez on 6/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#import "SilkAudioApp.h"
#import "IKLine.h"
#import "Segment.h"
#import "Cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
#pragma mark Setup
void SilkAudioApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize(1200, 800);
	settings->setFrameRate( 60 );
	settings->enableSecondaryDisplayBlanking(false);
}

void SilkAudioApp::setup()
{	
	std::cout << "Setting application path: " << getAppPath() << std::endl;
	chdir( getAppPath().c_str( ) );

	srandom( time(NULL) );
	ci::Rand::randSeed( random() );
	_perlinNoise.setSeed( sin( time( NULL ) ) * 10);
	_perlinNoise.setOctaves( 4 );

	_mousePosition = getWindowCenter();

	_gravity = 0.0;
	_glitchSegment = false;
	_drawPins = false;
	
	// Behavior modification
	_alphaWhenDrawing = 0.11;
	_mouseChaseDamping = 0.2;
	
	// state
	_state = kStateNormal;
    _additiveBlending = false;
	_useBezier = true;
	toggleAdditiveBlending( true );
	randomizeBrush();
	
	// PARAMS
	_drawParams = false;
	_params = ci::params::InterfaceGl( "Settings", ci::Vec2i( 200, 250 ) );
	_params.addParam("BristleCount", &_bristleCount, "min=1 max=100.0 step=1");
	_params.addParam("BrushRadius", &_brushRadius, "min=0 max=25.0 step=0.5");
	_params.addSeparator();
	_params.addParam("FilamentSpacing", &_filamentSpacing, "min=1.0 max=20.0 step=0.5");
	_params.addParam("FilamentCount", &_filamentCount, "min=5.0 max=100 step=5.0");
	_params.addSeparator();
	_params.addParam("FrictionMin", &_canvasFrictionMin, "min=0.2 max=0.9 step=0.01");
	_params.addParam("FrictionMax", &_canvasFrictionMax, "min=0.2 max=0.96 step=0.01");
	_params.addSeparator();
	_params.addParam("Gravity", &_gravity, "min=0.0 max=2.0 step=0.1");
	_params.addSeparator();
	_params.addParam("ChaseSpeed", &_mouseChaseDamping, "", true);
	_params.addParam("Alpha", &_alphaWhenDrawingFloat, "", true);
	
#ifdef __COLORMODE__TEST
	_params.addSeparator();
	_params.addParam("__R_LEFT", &__R_LEFT, "min=0.0 max=360.0 step=1");
	_params.addParam("__G_LEFT", &__G_LEFT, "min=0.0 max=1.0 step=0.01");
	_params.addParam("__B_LEFT", &__B_LEFT, "min=0.0 max=1.0 step=0.01");
	_params.addParam("__RGB_ALPHA", &__RGB_ALPHA, "min=0.0 max=1.0 step=0.001");
	_params.addParam("__R_RIGHT", &__R_RIGHT, "min=0.0 max=360.0 step=0.01");
	_params.addParam("__G_RIGHT", &__G_RIGHT, "min=0.0 max=1.0 step=0.01");
	_params.addParam("__B_RIGHT", &__B_RIGHT, "min=0.0 max=1.0 step=0.01");
	
	__R_LEFT = 0.4;
	__G_LEFT = 0.4;
	__B_LEFT = 0.4;
	__RGB_ALPHA = 0.065;
	__R_RIGHT = 0.5;
	__G_RIGHT = 0.5;
	__B_RIGHT = 0.5;
#endif
	
	_params.show( false ); 

	_instructionsBlack = ci::gl::Texture( loadImage( loadResource( INSTRUCTIONS_BLACK ) ) );
	_instructionsWhite = ci::gl::Texture( loadImage( loadResource( INSTRUCTIONS_WHITE ) ) );
	
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    audioSetup();
    count = 1;
}

void SilkAudioApp::toggleAdditiveBlending( bool enableAdditiveBlending )
{
	if(_additiveBlending == enableAdditiveBlending) return;
	
	// Change the blending mode and clear the screen
	if(enableAdditiveBlending)
	{
		_clearColor = ci::Colorf(0, 0, 0);
		ci::gl::enableAdditiveBlending();
		ci::gl::clear( _clearColor );
		
	} else {
		// alpha blending
		_clearColor = ci::Colorf(1, 1, 1);
		ci::gl::enableAlphaBlending();
		ci::gl::clear( _clearColor );
	}
	
	_additiveBlending = enableAdditiveBlending;
}

#pragma mark Events

void SilkAudioApp::mouseDown( ci::app::MouseEvent event )
{
	
	_oldAlphaWhenDrawing = _alphaWhenDrawing;
	//ci::gl::clear( _clearColor );
}

void SilkAudioApp::mouseDrag( ci::app::MouseEvent event )
{
	if(_state != kStateNormal) return;
	
	// Not drawing lines, clear the frame
	if(drawLines == false)
		ci::gl::clear( _clearColor );
	
	_alphaWhenDrawing = _oldAlphaWhenDrawing;
		
	// start drawing lines
	_mousePosition = event.getPos();
	drawLines = true;
}

void SilkAudioApp::mouseMove( ci::app::MouseEvent event )
{	
	_mousePosition = event.getPos();
}

void SilkAudioApp::mouseUp( ci::app::MouseEvent event )
{
	
	// Alpha when drawing becomes whatever is the highest (incase they put the mouse up before letting go of shift)
	_alphaWhenDrawing = ci::math<double>::max(_oldAlphaWhenDrawing, _alphaWhenDrawing);
	_oldAlphaWhenDrawing = _alphaWhenDrawing;
	
	//ci::gl::clear( _clearColor );
	//drawLines = false;
}

void SilkAudioApp::keyUp( ci::app::KeyEvent event )
{
	
}

void SilkAudioApp::keyDown( ci::app::KeyEvent event )
{
	if(_state != kStateNormal) return;
			
	char keyPressed = event.getChar();
	int colorMode;
	switch (keyPressed)
	{
		case 's':
			saveOutBrushImageAndParameters();
			break;
		case 'o':
			_drawParams = !_drawParams;
			_params.show( !_params.isVisible() );
			break;
		case 'r':
			randomizeBrush();
            count = 1;
			break;
		case 'm':
			createBrush();
			break;
		case 'g':
			_glitchSegment = !_glitchSegment;
			break;
		case 'l':
			_useBezier = !_useBezier;
		case 'p':
			_drawPins = !_drawPins;
			break;
		case '1': // COLORMODE_HSV
		case '2': // COLORMODE_RGB
		case '3': // COLORMODE_RGBINVERSE
		case '4': // COLORMODE_RGB_B
		case '5': // COLORMODE_RGB_C
		case '6': // COLORMODE_GRAYSCALE
		case '7': // COLORMODE_ALPHABLEND_1
		case '8': // COLORMODE_ALPHABLEND_2
			colorMode = boost::lexical_cast<int>( keyPressed );
			toggleAdditiveBlending( colorMode < COLORMODE_GRAYSCALE);
			_colorMode = colorMode;
			break;
		default:
			console() << keyPressed << std::endl;
			break;
	}
	
	// slowdown or speed up how fast our object chases the mouse - clamp addition of speed (determined by ternary operator)
	if(event.getCode() == ci::app::KeyEvent::KEY_LEFT || event.getCode() == ci::app::KeyEvent::KEY_RIGHT) {
		if(event.getCode() == ci::app::KeyEvent::KEY_LEFT)
			_mouseChaseDamping -= 0.02;
		else if(event.getCode() == ci::app::KeyEvent::KEY_RIGHT)
			_mouseChaseDamping += 0.02;
		
		_mouseChaseDamping = ci::math<float>::clamp(_mouseChaseDamping, 0.02, 1.0);
		updateParams();
	}
	
	if(event.getCode() == ci::app::KeyEvent::KEY_UP || event.getCode() == ci::app::KeyEvent::KEY_DOWN)
	{
		_alphaWhenDrawing = ci::math<double>::clamp(_alphaWhenDrawing + ((event.getCode() == ci::app::KeyEvent::KEY_UP) ? 0.005 : -0.005), 0.0, 1.0);
		updateParams();
	}
	
	// exit fullscreen with escape, and toggle with F
	if ( event.getChar() == 'f' || (isFullScreen() && event.getCode() == ci::app::KeyEvent::KEY_ESCAPE) )
	{
		ci::gl::clear(_clearColor);
		setFullScreen( !isFullScreen() );
	}
}

void SilkAudioApp::updateParams()
{
	if(_params.isVisible()) {
		_params.show( false );
		_params.show( true );
	}
}

#pragma mark File Handling
void SilkAudioApp::saveOutBrushImageAndParameters()
{
	using namespace boost::posix_time;

	// Use boost to figure out the current time on local machine
	ptime now = second_clock::local_time();

	// The time is nigh!
	std::stringstream ss;
	ss << now.date().month() <<  now.date().day() << "_";
	std::string timeStamp = ss.str();
	
	// Save the filename
	std::string fileName = ci::getHomeDirectory().string() + "SilkAudioApp/Brush_" + timeStamp;
	
	// Lets write the file now
	// Get an OStreamFileRef which is what you'll use to write to your file.
	ci::OStreamFileRef oStream = ci::writeFileStream( fileName + ".txt", true );
	
	// Create a string with the data you want to write.
	std::string myString = "SilkAudio parameters\n-\n";
	myString += "Parameters Used: [" + toString(_bristleCount) + "," + toString(_brushRadius) + "," + toString(_filamentSpacing) + "," + toString(_filamentCount);
	myString += ",0." + toString(round(_canvasFrictionMin*100));
	myString += ",0." + toString(round(_canvasFrictionMax*100));
	myString += ",0." + toString(round(_gravity * 100));
 	myString += "," + toString(_alphaWhenDrawing);
	myString += "," + toString(_mouseChaseDamping);
	myString +=+ "]";																																																																
	myString += "\n";
	
	// Human output of params used
	myString += "\t BristleCount: " + toString(_bristleCount) + " \n";
	myString += "\t BrushRadius: " + toString(_brushRadius) + " \n";
	myString += "\t FilamentSpacing: " + toString(_filamentSpacing) + " \n";
	myString += "\t FilimentCount: " + toString(_filamentCount) + " \n";
	myString += "\t FrictionMin: 0." + toString(round(_canvasFrictionMin * 100)) + " \n";
	myString += "\t FrictionMax: 0." + toString(round(_canvasFrictionMax * 100)) + " \n";
	myString += "\t Gravity: 0." + toString(round(_gravity * 100)) + " \n";
	myString += "\t Painting Alpha: " + toString(_alphaWhenDrawing) + " \n";
	myString += "\t Mousechase Damping: " + toString(_mouseChaseDamping) + " \n";
	
	trace( myString );
	// Now write the text to the file.
	oStream->writeData( myString.data(), myString.length() );
	

	// Write the image now that the directory exist
	writeImage( fileName + ".png", copyWindowSurface() );
}


void SilkAudioApp::fileDrop( ci::app::FileDropEvent event )
{
	if(_state != kStateNormal) return;
	
	if(event.getNumFiles() != 1) return;
	
	std::string fileRef = event.getFile(0).string();
	
	ci::IStreamFileRef stream = ci::loadFileStream(fileRef);
	
	while (stream->isEof() == false)
	{
		std::string currentLine = stream->readLine();
		std::string::size_type locationIndex = currentLine.find( "Parameters Used: [", 0 );
		
		// Found a match lets set the parameters
		if( locationIndex != std::string::npos )
		{
			setBrushSettingsFromStringParameters(currentLine);
			return;
		}
	}	
}

#pragma mark Brush Creation
void SilkAudioApp::randomizeBrush()
{	
	std::vector<std::string> randomParamsList;
	randomParamsList.push_back("Parameters Used: [23,9,8,15,0.78,0.85,0.0]");
	randomParamsList.push_back("Parameters Used: [24,9.5,7,30,0.83,0.87,0.0]");
	randomParamsList.push_back("Parameters Used: [100,0,4.5,45,0.75,0.82,0.0]");
	randomParamsList.push_back("Parameters Used: [100,22.5,1.5,30,0.77,0.77,0.0]");
	randomParamsList.push_back("Parameters Used: [100,25,1.5,30,0.77,0.77,0.50,0.059999999999999921,0.0400000066]");
	randomParamsList.push_back("Parameters Used: [9,1.5,4.5,45,0.74,0.76,0.0]");
	randomParamsList.push_back("Parameters Used: [23,5,1.5,45,0.76,0.83,0.0]");
	randomParamsList.push_back("Parameters Used: [12,2.5,2.5,65,0.72,0.79,0.13]");
	randomParamsList.push_back("Parameters Used: [5,0,2,55,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [5,0,2,55,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [8,0,2.5,85,0.8,0.883,0.0]");
	randomParamsList.push_back("Parameters Used: [31,0,4.5,25,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [8,0,2,55,0.84,0.86,0.0]");

	int i = ci::Rand::randInt( randomParamsList.size() );
	std::string randomParams = randomParamsList[i];
	
	console() << randomParams << std::endl;
	setBrushSettingsFromStringParameters( randomParams );
	_colorMode = ci::Rand::randInt(COLORMODE_HSV, COLORMODE_ALPHABLEND_3);
	toggleAdditiveBlending( _colorMode < COLORMODE_GRAYSCALE );
	createBrush();
}

void SilkAudioApp::setBrushSettingsFromStringParameters( std::string currentLine )
{
	// grab all the values from the string by splitting on ","
	std::vector<std::string> values;
	boost::split(values, currentLine, boost::is_any_of(","));
	
	console() << "Setting Strings from: " << currentLine << std::endl;
	if( values.size() != 7 && values.size() != 9) return; // more or less than 7, just exit file is no good
	for (int i = 0; i < values.size() ; i++)
	{
		std::vector<std::string> split;
		
		console() << values[i] << std::endl;
		// use boost::lexical_cast to convert the strings to the proper datatype
		switch (i)
		{
			case 0: // This one has the prefix above as part of its string
				boost::split(split, values[i], boost::is_any_of("["));
				_bristleCount = boost::lexical_cast<int>( split[1] );
				break;
			case 1:
				_brushRadius = boost::lexical_cast<float>( values[i] );
				break;
			case 2:
				_filamentSpacing = boost::lexical_cast<float>( values[i] );
				break;
			case 3:
				_filamentCount = boost::lexical_cast<float>( values[i] );
				break;
			case 4:
				_canvasFrictionMin = boost::lexical_cast<float>( values[i] );
				break;
			case 5:
				_canvasFrictionMax = boost::lexical_cast<float>( values[i] );
				break;
			case 6:
				split.clear();
				boost::split(split, values[i], boost::is_any_of("]"));
				
				if(split.size() > 0) {
					boost::split(split, values[i], boost::is_any_of("]"));
					_gravity = boost::lexical_cast<float>( split[0] );
				} else {
					_gravity = boost::lexical_cast<float>( values[i] );
				}
				break;
			case 7:
				_alphaWhenDrawing = boost::lexical_cast<double>( values[i] );
				break;
			case 8:
				boost::split(split, values[i], boost::is_any_of("]"));
				_mouseChaseDamping = boost::lexical_cast<float>( split[0] );
			default:
				break;
		};
	}
	
	updateParams();
	// Everythign went great - lets recreate recreate the brush
	createBrush();
}

void SilkAudioApp::createBrush()
{	
	if(_lines.size() != 0)
		_lines.clear();
	
	if(_canvasFrictionMin >= _canvasFrictionMax)
		_canvasFrictionMin = _canvasFrictionMax;
	
	for (int i = 0; i < _bristleCount; i++)
	{
		float radius = ci::Rand::randFloat() * _brushRadius;
		float radian = ci::Rand::randFloat() * M_PI * 2;
		
		//linePointer
		linePointer line = linePointer(new IKLine());
		
		line->x = cosf(radian) * radius;
		line->y = sinf(radian) * radius;
		
		line->segmentLength = _filamentSpacing;
		line->segmentNum = _filamentCount;
		line->gravity = _gravity;
		line->friction = ci::Rand::randFloat(_canvasFrictionMin, _canvasFrictionMax);
		line->init();
		
		_lines.push_back(line);
	}
}

#pragma mark Update / Draw 
void SilkAudioApp::update()
{
	
	float nX = (getWindowCenter().x) * 0.01f;
	float nY = (getWindowCenter().y) * 0.01f;
	float nZ = getElapsedFrames() * 0.003f;

	float mNoise = _perlinNoise.noise( nX, nY, nZ );

	float angle = mNoise * M_PI;

	ci::Vec2f mousePosition;
	bool doNoise = true;
	
	if (doNoise)
	{
		float yRange = getWindowHeight() * 0.4;
		ci::Vec2f noiseVector( cosf( angle ) * 100 + (_mousePosition.x*0.1) , sinf( angle ) * yRange + (_mousePosition.y*0.1));
		mousePosition = _mousePosition;
	} else {
		mousePosition = _mousePosition;
	}

	static float i;
	for( std::vector<linePointer>::iterator p = _lines.begin(); p != _lines.end(); ++p) {
		
		if(doNoise)
		{
			float localNoise = _perlinNoise.fBm( i*0.01f );
			localNoise*=5;
			
			ci::Vec2f localPos = mousePosition;
			localPos.x += cosf(localNoise) * 5;
			localPos.y += sinf(localNoise) * 5;
			(*p)->nextFrame(localPos, _mouseChaseDamping);
			
			i++;
		}
		else
		{
			(*p)->nextFrame(mousePosition, _mouseChaseDamping);
		}
	}

    xshift = getElapsedFrames() % getWindowWidth();
    if (xshift == 0.0f){
        count++;
    }
    audioUpdate();
}

void SilkAudioApp::draw()
{	
	_alphaWhenDrawingFloat = _alphaWhenDrawing;
	
	// Not drawing, clear screen
	if( drawLines == false ) {
		ci::gl::clear( _clearColor );
	} else { // overdraw to clear a little
//		ColorA stringColor = (_colorMode > COLORMODE_GRAYSCALE) ? ColorA(1.0, 1.0, 1.0, 1.0) : ColorA(0.0, 0.0, 0.0, 1.0);
//		glColor4f( stringColor.r, stringColor.g, stringColor.b, 0.01 );
//		gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight() ) );
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	for( std::vector<linePointer>::iterator p = _lines.begin(); p != _lines.end(); ++p) {
		drawIKLine( *p );
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	
	if(_drawParams) {
		ci::params::InterfaceGl::draw();
		ci::gl::Texture instructionsOverlay = (_colorMode < COLORMODE_GRAYSCALE) ? _instructionsBlack : _instructionsWhite;
		
		if(instructionsOverlay)
		{
			//gl::clear( Color( 0, 0, 0 ), false );
			
			glColor4f( 1.0, 1.0, 1.0, 1.0 );
			glEnable( GL_TEXTURE_2D );
			ci::gl::draw( instructionsOverlay, ci::Vec2f(10, getWindowHeight() - instructionsOverlay.getHeight() - 25 ) );
			glDisable( GL_TEXTURE_2D );	
			
			ci::gl::drawSolidRect(ci::Rectf(0, getWindowHeight()-25 , instructionsOverlay.getWidth(), getWindowHeight()) );
			//glColor4f( 1.0, 1.0, 1.0, 1.0 );
		}
	}
}


void SilkAudioApp::drawIKLine( linePointer& lineToDraw )
{		
	ci::Vec2f start = ci::Vec2f::zero();
	start.x = lineToDraw->x + lineToDraw->_segments[0]->x;
	start.y = lineToDraw->y + lineToDraw->_segments[0]->y;
	
	double alpha;
	
	// If drawing lines, h
	if(drawLines)
	{
		if(_colorMode <= COLORMODE_GRAYSCALE) alpha = _alphaWhenDrawing;
		else alpha = _alphaWhenDrawing*.55;
	} 
	else {
		alpha = 1.0;
	}
	
	for (int i = 0; i < lineToDraw->segmentNum - 2; i++)
	{
		ci::Vec2f end = ci::Vec2f::zero();
		end.x = (lineToDraw->x + lineToDraw->_segments[i]->x + lineToDraw->x + lineToDraw->_segments[i+1]->x ) * 0.5;
		end.y = (lineToDraw->y + lineToDraw->_segments[i]->y + lineToDraw->y + lineToDraw->_segments[i+1]->y ) * 0.5;
		
		ci::Vec2f control = ci::Vec2f::zero();
		control.x = lineToDraw->x + lineToDraw->_segments[i]->x;
		control.y = lineToDraw->y + lineToDraw->_segments[i]->y;
		
		
		// Get the color based on distance in array from line
		double nonInverseI = ( (double)i / (double)(lineToDraw->segmentNum-2) );
		double inverseI = 1.0f - nonInverseI;
		
		getColorMode( inverseI, alpha);
		glLineWidth(inverseI);
		
		Segment segment = *(lineToDraw->_segments[i]);
		if(_useBezier)
		{
			if(_drawPins)
			{
				ci::Vec2f pin = segment.getPin();
				ci::gl::drawSolidCircle(ci::Vec2f(lineToDraw->x, lineToDraw->y) + pin, 2, 6);
				
				glEnableClientState(GL_VERTEX_ARRAY);
				drawBezier(start, control, end, 5);	
				glDisableClientState(GL_VERTEX_ARRAY);
			} else {
				drawBezier(start, control, end, 10);
			}
		}
		else  // draw straight lines
		{
			if(_drawPins) 
			{
				ci::Vec2f pin = segment.getPin();
				ci::gl::drawSolidCircle(ci::Vec2f(lineToDraw->x, lineToDraw->y) + pin, 2, 6);
			}
			
			glEnableClientState(GL_VERTEX_ARRAY);
			ci::Vec2f vertices[2];
			vertices[0] = start;
			vertices[1] = end;
			
			//glVertexPointer(2, GL_FLOAT, 0, vertices);
			//glDrawArrays(GL_LINES, 0, 2);
			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glDrawArrays(GL_LINE_STRIP, 0, 2);
			glDisableClientState(GL_VERTEX_ARRAY);

		}
		
		start = end;
	}
}

inline void SilkAudioApp::getColorMode(double inverseI, double alpha)
{
// Use when creating new color modes with ParamsGui
#ifdef __COLORMODE__TEST	
	double defaultAlpha = 0.8;
	glColor4d((__R_LEFT + (inverseI * __R_RIGHT)),
			  (__G_LEFT + (cos(inverseI * M_PI * 2) * __G_RIGHT)),
			  (__B_LEFT + (sin(inverseI * M_PI * 2) * __B_RIGHT)), 
			  inverseI * alpha);
	return;
#endif
	
	// color mode inverse
	float left;
	float right;
	float r,g,b;
	switch (_colorMode)
	{
		case COLORMODE_HSV: // 1
			HSVtoRGB( &r, &g, &b, 101.0 + cos(inverseI * M_PI * 2) * 300.0, ci::math<float>::max(0.89 + sin(inverseI * M_PI * 2) * 0.51, 1.0), 0.05 );
			glColor4d(r,g,b, inverseI * alpha * 1.5);
			break;
		case COLORMODE_RGB: // 2
			glColor4d((0.3 + (inverseI * 0.25)),
					  (0.3 + (cos(inverseI * M_PI * 2) * 0.25)),
					  (0.3 + (sin(inverseI * M_PI * 2) * 0.25)), 
					  inverseI * alpha);
			break;
		case COLORMODE_RGBINVERSE: // 3 
			left = 0.35;
			right = 0.45;
			glColor4d(0.9 - (left + (inverseI * right)),
					  0.9 - (left + (sin(inverseI * M_PI * 2) * right)),
					  0.9 - (left + (cos(inverseI * M_PI * 2) * right)), 
					  inverseI * alpha*0.8);	
			break;
		case COLORMODE_RGB_B: // 4
			glColor4d(0.0 + inverseI * 0.25,
					  0.0 + cos(inverseI * M_PI * 2) * 0.1,
					  0.0, 
					  inverseI * alpha);
			break;	
		case COLORMODE_RGB_C: // 5
			glColor4f( 0.7f + (inverseI * 0.5f),
					  0.7f + (sinf(inverseI * M_PI * 2) * 0.5f),
					  0.7f + (cosf(inverseI * M_PI * 2) * 0.5f), 
					  inverseI * 0.1 );
			break;
			
		case COLORMODE_GRAYSCALE: // 6
			glColor4d(0.25, 0.25, 0.25, inverseI * alpha * 0.5);
			break;
		case COLORMODE_ALPHABLEND_1: // 7
			left = 0.4;
			right = 0.5;
			glColor4d(1.0 - (left + (inverseI * right)),
					  1.0 - (left + (sin(inverseI * M_PI * 2) * right)),
					  1.0 - (left + (cos(inverseI * M_PI * 2) * right)), 
					  inverseI * alpha );	
			break;
		case COLORMODE_ALPHABLEND_2: // 8
			glColor4d(0.83 + (inverseI * 0.4),
					  0.4 + (cos(inverseI * M_PI * 2) * 0.44),
					  (0.37 + (sin(inverseI * M_PI * 2) * 0.4)),
					  inverseI * alpha );
			break;
		case COLORMODE_ALPHABLEND_3: // 9
			glColor4d((0.4 + (sin(inverseI * M_PI * 2) * 0.32)),
					  (0.23 + (cos(inverseI * M_PI * 2) * 0.25)),
					  (0.5 + (inverseI * 0.21)),
					  inverseI * alpha );
			break;
		default:
			left = 0.4;
			right = 0.5;
			glColor4d(1.0 - (left + (inverseI * right)),
					  1.0 - (left + (sin(inverseI * M_PI * 2) * right)),
					  1.0 - (left + (cos(inverseI * M_PI * 2) * right)), 
					  inverseI * alpha );	
			break;
	}	
}

inline void SilkAudioApp::drawBezier(ci::Vec2f origin, ci::Vec2f control, ci::Vec2f destination, const int segments)
{
	ci::Vec2f vertices[11];
	
	float t = 0.0;
	for(int i = 0; i <= segments; i++)
	{
		float x = pow(1.0f - t, 2) * origin.x + 2.0f * (1.0f - t) * t * control.x + t * t * destination.x;
		float y = pow(1.0f - t, 2) * origin.y + 2.0f * (1.0f - t) * t * control.y + t * t * destination.y;
		vertices[i] = ci::Vec2f(x, y);
		t += 1.0f / segments;
	}
	vertices[segments] = destination;

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_LINE_STRIP, 0, segments + !_glitchSegment);
}

template <class T> inline std::string SilkAudioApp::toString (const T& t)
{
	std::setprecision(2);
	return boost::lexical_cast<std::string>( t );
}

void SilkAudioApp::audioUpdate()
{
	mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}
	uint16_t bandCount = 512;
	//presently FFT only works on OS X, not iOS
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
    float * fftBuffer = mFftDataRef.get();
    
    if (fftBuffer)
        volume = fftBuffer[0]*300.0;
    
    _mousePosition = Vec2f(xshift,volume+count*100);
    
}

void SilkAudioApp::audioSetup()
{
	//iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
	mInput = audio::Input();
	
	//tell the input to start capturing audio
	mInput.start();
	
}




CINDER_APP_BASIC( SilkAudioApp, ci::app::RendererGl )
