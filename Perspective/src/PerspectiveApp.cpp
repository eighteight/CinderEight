#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PerspectiveApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

    void beginProjection();  
	void endProjection();
    
	void drawGrid();  
	void drawCircles();  
	void drawBox(double _x, double _y, double _z, double _w, double _h, double _d, Color _clr, double _ry);
	void drawFloor(double _x, double _z, double _size, Color _clr);
    float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp);
    void ofBox(float);
    
	double headX, headY, headZ;  
	double fNear, fFar, fFov;  
	double mX, mY, rY;
    
	double elapsed;
	double quadX;
	double quadZ;
	Color quadC;
    
	vec3 objPos[10];
	Color colors[10][4];
    
	Capture mCapture;
    
    gl::TextureRef mTexture;
    
	bool bVideo, bBoxes, bCircles, bFloor, bGrid, bHelp;
    
};

void PerspectiveApp::setup()
{
    bVideo = bBoxes = bCircles = bFloor = bGrid = bHelp = true;
    
	elapsed = 0.0;	// for animation
	quadX = 0.3;	// floor X
	quadZ = 0.3;	// floor Y
	rY = 0.0;		// rotation

	try {
		mCapture = Capture( 640, 480 );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
    
	for(int i = 0; i < 10; i++) {
		double _x = Rand::randFloat( 3.0f, 13.0f );
		double _y = Rand::randFloat( 3.0f, 13.0f );
		double _z = Rand::randFloat( 3.0f, 13.0f );
        
		objPos[i] = vec3(_x,_y,_z);
        
		for(int j = 0; j < 4; j++) {
			Color _color;
//			_color.set((unsigned char)255.0*Rand::randFloat(0.0f,1.0f), (unsigned char)255.0*Rand::randFloat(0.0f,1.0f), (unsigned char)255.0*Rand::randFloat(0.0f,1.0f), (unsigned char)255.0*0.4);
			colors[i][j] = _color;
		}
	}
}

void PerspectiveApp::mouseDown( MouseEvent event )
{
}

void PerspectiveApp::update()
{
    // for animation
	elapsed += 0.0f;//getLpseFrameTime();
	if(elapsed > 1.0) elapsed = 0.0;
    
	//rY += 0.5 / ofGetFrameRate(); // angular step - Y axis
	if(rY > 360.0) rY -= 360.0;
    
    if( mCapture && mCapture.checkNewFrame() ) {
        mTexture = gl::Texture::create( mCapture.getSurface() );
	}
}

void PerspectiveApp::drawFloor(double _x, double _z, double _size, Color _clr) {
	double _y = -1.0;
    gl::color(_clr.r, _clr.g, _clr.b);
	glBegin(GL_QUADS);
    gl::vertex(_x,		 _y,	_z);
    gl::vertex(_x+_size, _y,	_z);
    gl::vertex(_x+_size, _y,	_z+_size);
    gl::vertex(_x,		 _y,	_z+_size);
	glEnd();
	gl::color((_clr*1.2).r, (_clr*1.2).g, (_clr*1.2).b);
	glBegin(GL_QUADS);
    gl::vertex(_x,		 _y,	_z);
    gl::vertex(_x+_size, _y,	_z);
    gl::vertex(_x+_size, _y,	_z+_size);
    gl::vertex(_x,		 _y,	_z+_size);
	glEnd();
}

void PerspectiveApp::beginProjection(){ 
        
        gl:MatrixMode(GL_PROJECTION);
        glPushMatrix();  
        glMatrixMode(GL_MODELVIEW);  
        glPushMatrix(); 
        
        fNear = 0.5f;  
        fFar = 1000.0f;  
        fFov = tan(30 * M_PI / 360);  fFov = 0.5;
        
        float ratio =getWindowWidth() / getWindowHeight();  
        
        double _mx = (double)getMousePos().x;
        double _my = (double)getMousePos().y;
        double _w  = (double)getWindowWidth();
        double _h  = (double)getWindowHeight();
        
        headX = (_mx / _w) - 0.5;  
        headY = ((_h - _my) / _h) - 0.5;  
        headZ = -2.0;
        
        glMatrixMode(GL_PROJECTION);  
        glLoadIdentity();  
        
        glFrustum(fNear*(-fFov * ratio + headX),  
                  fNear*(fFov * ratio + headX),  
                  fNear*(-fFov + headY),  
                  fNear*(fFov + headY),  
                  fNear, fFar);  
        
        glMatrixMode(GL_MODELVIEW);  
        glLoadIdentity();  
        //gluLookAt(headX*headZ, headY*headZ, 0, headX*headZ, headY*headZ, -1, 0, 1, 0);
        glTranslatef(0.0,0.0,headZ);
}

void PerspectiveApp::drawGrid(){
    int nbOfSteps = 10;  
	float frameW = 1.0;  
	float frameH = 1.0;  
    
	//ofPushStyle();  
	for (int i = 0; i < nbOfSteps; i++) {  
		if(nbOfSteps - i == (int)ofMap(elapsed, 0.0, 1.0, 1, nbOfSteps, true)) {
            glLineWidth(10);
			glColor4i(160,255,160,80);  
		} else {
			glLineWidth(1);
			gl::color(255,255,255,80);
		}
        	gl::pushMatrices();  
		{  
            gl::translate(.0f, .0f, i * -0.1f);  
            gl::drawLine(vec2(-frameW, -frameH), vec2(frameW, -frameH));
            gl::drawLine(vec2(frameW, -frameH), vec2(frameW,  frameH));
            gl::drawLine(vec2(frameW,  frameH), vec2(-frameW,  frameH));
			gl::drawLine(vec2(-frameW,  frameH), vec2(-frameW, -frameH));
		}  
			gl::popMatrices();  
	}  
    gl::color(255,255,255,80);
	glLineWidth(1);
	for (int i = 0; i < nbOfSteps*2; i++) {  
		double _x = frameW/(double)nbOfSteps*(double)i-1.0;
		double _y = frameH/(double)nbOfSteps*(double)i-1.0;
		gl::drawLine(vec3(-_x, -frameH, 0.0), vec3(-_x, -frameH, -1.0));  
		gl::drawLine(vec3( _x,  frameH, 0.0),vec3( _x,  frameH, -1.0));  
		gl::drawLine(vec3( frameW, -_y, 0.0), vec3(frameW, -_y, -1.0));  
		gl::drawLine(vec3(-frameW,  _y, 0.0), vec3(-frameW,  _y, -1.0));
	}  
	//ofPopStyle();
}

float PerspectiveApp::ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
    
	if (fabs(inputMin - inputMax) < FLT_EPSILON){

		return outputMin;
	} else {
		float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
        
		if( clamp ){
			if(outputMax < outputMin){
				if( outVal < outputMax )outVal = outputMax;
				else if( outVal > outputMin )outVal = outputMin;
			}else{
				if( outVal > outputMax )outVal = outputMax;
				else if( outVal < outputMin )outVal = outputMin;
			}
		}
		return outVal;
	}
    
}

void PerspectiveApp::drawCircles() {  
	glLineWidth(3);
	for(int i = 0; i < 10; i++) {
		for(int j = 0; j < 4; j++) {
			gl::color(colors[i][j]);
            gl::drawSolidCircle(vec2(objPos[i].x, objPos[i].y), 0.2-0.04*(double)j);
			gl::color(255,255,0,255);
            gl::drawLine(objPos[i], vec3(objPos[i].x, objPos[i].y, objPos[i].z + 1.0));
		}
	}
	glLineWidth(1);
}

void PerspectiveApp::drawBox(double _x, double _y, double _z, double _w, double _h, double _d, Color _clr, double _ry) {
	glPushMatrix();
    gl::translate(_x, _y, _z);
    glRotated(_ry * 360.0, 0,1,0);
    gl::scale(_w, _h, _d);
	ofFill();
	glColor4f(_clr);
	ofBox(1.0);
	ofNoFill();
	glColor4f(_clr*1.2);
	ofBox(1.0);
	glPopMatrix();
}

void PerspectiveApp::ofBox(float size){
	glPushMatrix();
//	if(ofGetCoordHandedness() == OF_LEFT_HANDED){
//		ofScale(1, 1, -1);
//	}
    
	float h = size * .5;
	
	vertexData.clear();
		vec3 vertices[] = {
			vec3(+h,+h,+h),
			vec3(+h,+h,-h),
			vec3(+h,-h,+h),
			vec3(+h,-h,-h),
			vec3(-h,+h,+h),
			vec3(-h,+h,-h),
			vec3(-h,-h,+h),
			vec3(-h,-h,-h)
		};
		vertexData.addVertices(vertices,8);
		
		static float n = sqrtf(3);
		static vec3 normals[] = {
			vec3(+n,+n,+n),
			vec3(+n,+n,-n),
			vec3(+n,-n,+n),
			vec3(+n,-n,-n),
			vec3(-n,+n,+n),
			vec3(-n,+n,-n),
			vec3(-n,-n,+n),
			vec3(-n,-n,-n)
		};
		vertexData.addNormals(normals,8);
        
		static ofIndexType indices[] = {
			0,1, 1,3, 3,2, 2,0,
			4,5, 5,7, 7,6, 6,4,
			0,4, 5,1, 7,3, 6,2
		};
		vertexData.addIndices(indices,24);
        
		vertexData.setMode(OF_PRIMITIVE_LINES);
		renderer->draw(vertexData, vertexData.usingColors(),vertexData.usingTextures(),vertexData.usingNormals());
    
    
	glPopMatrix();
}


void PerspectiveApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableAlphaBlending();
    
	// calculate and apply perspective
	beginProjection(); 
    
    //--- grid -------------------------------------
    if(bGrid) drawGrid();
    
    if(bVideo) {//--- live camera -------------------------------------
        double _x = -1.0; double _y = -1.0; double _z = -1.0; double _w = 2.0; double _h = 2.0;
        double _tx = 320.0;	double _ty = 240.0;	double _tw = -320.0; double _th = -240.0;
        
        //glColor(255,255,255,255);
        mTexture.bind();
        glBegin(GL_QUADS);
        if(bGrid) {
            glTexCoord2f(_tx, _ty);			glVertex3f(_x,		_y,		_z);
            glTexCoord2f(_tx+_tw, _ty);		glVertex3f(_x+_w,	_y,		_z);
            glTexCoord2f(_tx+_tw,_ty+_th);	glVertex3f(_x+_w,	_y+_h,	_z);
            glTexCoord2f(_tx,_ty+_th);		glVertex3f(_x,		_y+_h,	_z);
        } else {
            glTexCoord2f(_tx, _ty);			glVertex3f(_x-_w*0.5,	_y-_h*0.5,	_z);
            glTexCoord2f(_tx+_tw, _ty);		glVertex3f(_x+_w*1.5,	_y-_h*0.5,	_z);
            glTexCoord2f(_tx+_tw,_ty+_th);	glVertex3f(_x+_w*1.5,	_y+_h*1.5,	_z);
            glTexCoord2f(_tx,_ty+_th);		glVertex3f(_x-_w*0.5,	_y+_h*1.5,	_z);
        }
        glEnd();
        mTexture.unbind();
    }
    
    if(bFloor) {//--- floor animation -------------------------------------
        if(elapsed == 0.0) {
            quadX = (int)Rand::randFloat(0.0, 10.0);
            quadX = -1.0 + 0.2 * quadX;
            quadZ = (int)Rand::randFloat(0.0, 5.0);
            quadZ = -0.2 -0.2 * quadZ;
            quadC = Color(Rand::randFloat(0,255), Rand::randFloat(0,255), Rand::randFloat(0,255));
        }
        drawFloor(quadX, quadZ, 0.2, quadC);
    }
    
    //--- circles -------------------------------------
    if(bCircles) drawCircles();
    
    if(bBoxes) {//--- boxes -------------------------------------
        double _s = 0.3;
        drawBox(0.0, -1.0+_s*0.5, -0.5, _s, _s, _s, Color(190, 190, 0), rY);
        _s = 0.15;
        drawBox(0.0, -1.0+_s*0.5, -0.5, _s, _s, _s, Color(40, 40, 19), 360.0-rY);
        
        double _w = 0.8; double _h = 0.8;
        drawBox(-1.0+_w*0.5, -0.3, -0.7, _w, 0.1, 0.1, Color(0, 190, 190), 0.0);
        drawBox(-0.5, -1.0+_h*0.5, -0.2, 0.1, _h, 0.1, Color(40, 190, 40), 0.0);
        drawBox(0.5, 1.0-_h*0.5, -0.2, 0.1, _h, 0.1, Color(190, 10, 190), 0.0);
        drawBox(1.0-_w*0.5, -0.6, -0.2, _w, 0.1, 0.1, Color(190, 10, 10), 0.0);
    }
    
	endProjection();
    
    gl::disableAlphaBlending();
    
	if(bHelp) {
//		ofSetColor(255,255,255,255);
//		ofDrawBitmapString("headX:" + ofToString(headX, 4), 10, 15);
//		ofDrawBitmapString("headY:" + ofToString(headY, 4), 10, 30);
//		ofDrawBitmapString("headZ:" + ofToString(headZ, 4), 10, 45);
//        
//		ofDrawBitmapString("(H)elp    on/off", 10,  85);
//		ofDrawBitmapString("(V)ideo   on/off", 10, 100);
//		ofDrawBitmapString("(G)rid    on/off", 10, 115);
//		ofDrawBitmapString("(C)ircles on/off", 10, 130);
//		ofDrawBitmapString("(B)oxes   on/off", 10, 145);
//		ofDrawBitmapString("(F)loor   on/off", 10, 160);
	}
}

CINDER_APP_BASIC( PerspectiveApp, RendererGl )
