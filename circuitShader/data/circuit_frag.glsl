#version 110
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;          // input channel. XX = 2D/Cube

#define time iGlobalTime*.02


#define width .005
float zoom = .18;
float curvtr = 1.0;

float shape=0.;
vec3 color=vec3(0.),randcol;

void formula(vec2 z, float c) {
	float minit=0.;
	float o,ot2,ot=ot2=1000.;
	for (int i=0; i<9; i++) {
		z=abs(z)/clamp(dot(z,z),.1,.5)-c;
		float l=length(z);
		o=min(max(abs(min(z.x,z.y)),-l+.25),abs(l-.25));
		ot=min(ot,o);
		ot2=min(l*.1,ot2);
		minit=max(minit,float(i)*(1.-abs(sign(ot-o))));
	}
	minit+=1.;
	float w=width*minit*2.;
	float circ=pow(max(0.,w-ot2)/w,6.);
	shape+=max(pow(max(0.,w-ot)/w,.25),circ);
	vec3 col=normalize(.1+texture2D(iChannel1,vec2(minit*.1)).rgb);
	color+=col*(.4+mod(minit/9.-time*10.+ot2*2.,1.)*1.6);
	color+=vec3(1.,.7,.3)*circ*(10.-minit)*3.*smoothstep(0.,.5,.15+texture2D(iChannel0,vec2(.0,1.)).x-.5);
}


void main(void)
{
	vec2 pos = gl_FragCoord.xy / iResolution.xy - .5;
	pos.x*=iResolution.x/iResolution.y;
	vec2 uv=pos;
	float sph = length(uv); sph = sqrt(1. - sph*sph)*curvtr; // curve for spheric distortion

	uv=normalize(vec3(uv,sph)).xy;
	float a=time+mod(time,1.)*.5;
	vec2 luv=uv;
	float b=a*5.48535;
    //	zoom*=1.+sin(time*3.758123)*.8;
    
	uv*=mat2(cos(b),sin(b),-sin(b),cos(b));  //mov
	uv+=vec2(sin(a),cos(a*.5))*8.;           //rotate
	uv*=zoom;                                //zoom
	float pix=.5/iResolution.x*zoom/sph;
	float dof=max(1.,(10.-mod(time,1.)/.01));
	float c=1.5+mod(floor(time),6.)*.125;
	for (int aa=0; aa<36; aa++) {
		vec2 aauv=floor(vec2(float(aa)/6.,mod(float(aa),6.)));
		formula(uv+aauv*pix*dof,c);
	}
	shape/=36.; color/=36.;
	vec3 colo=mix(vec3(.15),color,shape)*(1.-length(pos))*min(1.,abs(.5-mod(time+.5,1.))*10.);
	colo*=vec3(1.2,1.1,1.0);
	gl_FragColor = vec4(colo,1.0);
}