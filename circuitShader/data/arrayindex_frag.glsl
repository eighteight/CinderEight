#version 110
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform sampler2D iChannel0;          // sound
uniform float zoomm;
//uniform sampler2D iChannel1;          // palette

uniform sampler2DRect iChannel1;

// A trick to use any variable as an array index.
// Array index must be a constant, but the loop index is constant when unrolled
// so we can use it to compare it with any variable and return the array value that matches,
// or an interpolation of the two closest matches.

// mouse click to see only the fractal

vec3 palette[7]; // the color palette is stored here


// get a gradient of the palette based on c value, with a "smooth" parameter (0...1)
vec3 getsmcolor(float c, float smooth)
{
    smooth*=.5;
    c=mod(c-.5,7.);
    vec3 color1=vec3(0.0),color2=vec3(0.0);
    for(int i=0;i<7;i++) {
        if (float(i)-c<=.0) {
            color1 = palette[i];
            color2 = palette[(i+1>6)?0:i+1];
        }
    }
    // smooth mix the two colors
    return mix(color1,color2,smoothstep(.5-smooth,.5+smooth,fract(c)));
}
void main(void)
{
	// define the colors (rainbow)
	palette[6]=vec3(255,000,000)/255.;
	palette[5]=vec3(255,127,000)/255.;
	palette[4]=vec3(255,255,000)/255.;
	palette[3]=vec3(000,255,000)/255.;
	palette[2]=vec3(000,000,255)/255.;
	palette[1]=vec3(075,000,130)/255.;
	palette[0]=vec3(143,000,255)/255.;
	
	vec3 color=vec3(0.);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
	color=getsmcolor(uv.y*7.+iGlobalTime*.5,.25+.75*abs(sin(iGlobalTime))); //gradient function
    
	vec2 p=(uv-.5);
	p.x*=iResolution.x/iResolution.y;

	// fractal
	float a=iGlobalTime*.05;
	float ot,otl=ot=1000.;
	mat2 rot=mat2(cos(a),sin(a),-sin(a),cos(a));
	for(int i=0;i<14;i++) {//complexity
		p=abs(p+1.)-1.;
		p=p*1.25;
		p*=rot;
		ot=min(ot,abs(min(abs(p.y),abs(p.x))-.75)); //orbit trap 1
		ot=max(ot,length(p)*.02); //orbit trap 2
		otl=min(otl,abs(length(p)-.75)); //orbit trap 3
	}
	ot=pow(max(0.,1.-ot),10.); //orbit trap (0 to 1)
    
    color=getsmcolor(ot*7.+length(uv)*14.,1.); //get color gradient for orbit trap value
    color=mix(vec3(length(color)),color,.8)-pow(max(0.,1.-otl),25.)*.8+.1;
	
	gl_FragColor = vec4(color,1.0);
}