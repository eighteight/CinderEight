#version 150 core
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
#ifdef GL_ES
precision highp float;
#endif

//#define EPS		.001
//#define PI		3.14159265359
//#define RADIAN	180. / PI
//#define SPEED	iGlobalTime * 1.2
//
//out vec4 oColor;
//
//float hash(vec2 p)
//{
//    return fract(sin(dot(p,vec2(127.1,311.7))) * 43758.5453123);
//}
//
//float noise(vec2 p)
//{
//    vec2 i = floor(p), f = fract(p);
//	f *= f*(3.-2.*f);
//    return mix(mix(hash(i + vec2(0.,0.)),
//                   hash(i + vec2(1.,0.)), f.x),
//               mix(hash(i + vec2(0.,1.)),
//                   hash(i + vec2(1.,1.)), f.x), f.y);
//}
//
//float fbm(in vec2 p)
//{
//	return	.5000 * noise(p)
//    +.2500 * noise(p * 2.)
//    +.1250 * noise(p * 4.)
//    +.0625 * noise(p * 8.);
//}
//
//float dst(vec3 p)
//{
//	return dot(vec3(p.x, p.y + .45 * fbm(p.zx), p.z), vec3(0.,1.,0.));
//}
//
//vec3 nrm(vec3 p, float d)
//{
//	return normalize(
//                     vec3(dst(vec3(p.x + EPS, p.y, p.z)),
//                          dst(vec3(p.x, p.y + EPS, p.z)),
//                          dst(vec3(p.x, p.y, p.z + EPS))) - d);
//}
//
//bool rmarch(vec3 ro, vec3 rd, out vec3 p, out vec3 n)
//{
//	p = ro;
//	vec3 pos = p;
//	float d = 1.;
//    
//	for (int i = 0; i < 64; i++) {
//		d = dst(pos);
//        
//		if (d < EPS) {
//			p = pos;
//			break;
//		}
//		pos += d * rd;
//	}
//	
//	n = nrm(p, d);
//	return d < EPS;
//}
//
//void main(void)
//{
//	vec2 uv = gl_FragCoord.xy / iResolution.xy;
//	vec2 uvn = (2. * uv - 1.)
//    * vec2(iResolution.x / iResolution.y, 1.);
//	
//	if (abs(EPS + uvn.y) >= .7 || mod(floor(gl_FragCoord.y),2.) > 0.) {
//		oColor = vec4(vec3(0.),1.);
//		return;
//	}
//    
//	vec3 cu = vec3(0.,1.,0.);
//	vec3 cp = vec3(0., 1.1 + fbm(vec2(iGlobalTime)) * .2, SPEED);
//	vec3 ct = vec3(1.5 * sin(iGlobalTime),
//				   -8. + cos(iGlobalTime) + fbm(cp.xz) * 5., 15. + SPEED);
//    
//	vec3 ro = cp,
//    rd = normalize(vec3(uvn, 1. / tan(30. * RADIAN)));
//	
//	vec3 cd = ct - cp,
//    rz = normalize(cd),
//    rx = normalize(cross(rz, cu)),
//    ry = normalize(cross(rx, rz));
//    
//	rd = normalize(mat3(rx, ry, rz) * rd);
//    
//	vec3 sp, sn;
//	vec3 col = (rmarch(ro, rd, sp, sn) ?
//                vec3(.6) * dot(sn, normalize(vec3(cp.x, cp.y + .5, cp.z) - sp))
//                : vec3(0.));
//	
//	col += hash(hash(uv) * uv * iGlobalTime) * .15;
//	col *= 1.9 * smoothstep(length(uv * .5 - .25), .8, .4);
//	col *= smoothstep(EPS, 3.5, iGlobalTime);
//	oColor = vec4(col, 1.);
//}


// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Volumetric clouds. It performs level of detail (LOD) for faster rendering and antialiasing

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return -1.0+2.0*mix( rg.x, rg.y, f.z );
}

float map5( in vec3 p )
{
	vec3 q = p - vec3(0.0,0.1,1.0)*iGlobalTime;
	float f;
    f  = 0.50000*noise( q ); q = q*2.02;
    f += 0.25000*noise( q ); q = q*2.03;
    f += 0.12500*noise( q ); q = q*2.01;
    f += 0.06250*noise( q ); q = q*2.02;
    f += 0.03125*noise( q );
	return clamp( 1.5 - p.y - 2.0 + 1.75*f, 0.0, 1.0 );
}

float map4( in vec3 p )
{
	vec3 q = p - vec3(0.0,0.1,1.0)*iGlobalTime;
	float f;
    f  = 0.50000*noise( q ); q = q*2.02;
    f += 0.25000*noise( q ); q = q*2.03;
    f += 0.12500*noise( q ); q = q*2.01;
    f += 0.06250*noise( q );
	return clamp( 1.5 - p.y - 2.0 + 1.75*f, 0.0, 1.0 );
}
float map3( in vec3 p )
{
	vec3 q = p - vec3(0.0,0.1,1.0)*iGlobalTime;
	float f;
    f  = 0.50000*noise( q ); q = q*2.02;
    f += 0.25000*noise( q ); q = q*2.03;
    f += 0.12500*noise( q );
	return clamp( 1.5 - p.y - 2.0 + 1.75*f, 0.0, 1.0 );
}
float map2( in vec3 p )
{
	vec3 q = p - vec3(0.0,0.1,1.0)*iGlobalTime;
	float f;
    f  = 0.50000*noise( q ); q = q*2.02;
    f += 0.25000*noise( q );;
	return clamp( 1.5 - p.y - 2.0 + 1.75*f, 0.0, 1.0 );
}

vec3 sundir = normalize( vec3(-1.0,0.0,-1.0) );

vec4 integrate( in vec4 sum, in float dif, in float den, in vec3 bgcol, in float t )
{
    // lighting
    vec3 lin = vec3(0.65,0.68,0.7)*1.3 + 0.5*vec3(0.7, 0.5, 0.3)*dif;
    vec4 col = vec4( mix( 1.15*vec3(1.0,0.95,0.8), vec3(0.65), den ), den );
    col.xyz *= lin;
    col.xyz = mix( col.xyz, bgcol, 1.0-exp(-0.003*t*t) );
    // front to back blending
    col.a *= 0.4;
    col.rgb *= col.a;
    return sum + col*(1.0-sum.a);
}

#define MARCH(STEPS,MAPLOD) for(int i=0; i<STEPS; i++) { vec3  pos = ro + t*rd; if( pos.y<-3.0 || pos.y>2.0 || sum.a > 0.99 ) break; float den = MAPLOD( pos ); if( den>0.01 ) { float dif =  clamp((den - MAPLOD(pos+0.3*sundir))/0.6, 0.0, 1.0 ); sum = integrate( sum, dif, den, bgcol, t ); } t += max(0.1,0.02*t); }

vec4 raymarch( in vec3 ro, in vec3 rd, in vec3 bgcol )
{
	vec4 sum = vec4(0.0);
    
	float t = 0.0;
    
    MARCH(30,map5);
    MARCH(30,map4);
    MARCH(30,map3);
    MARCH(30,map2);
    
    return clamp( sum, 0.0, 1.0 );
}

mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

vec4 render( in vec3 ro, in vec3 rd )
{
    // background sky
	float sun = clamp( dot(sundir,rd), 0.0, 1.0 );
	vec3 col = vec3(0.6,0.71,0.75) - rd.y*0.2*vec3(1.0,0.5,1.0) + 0.15*0.5;
	col += 0.2*vec3(1.0,.6,0.1)*pow( sun, 8.0 );
    
    // clouds
    vec4 res = raymarch( ro, rd, col );
    col = col*(1.0-res.w) + res.xyz;
    
    // sun glare
	col += 0.1*vec3(1.0,0.4,0.2)*pow( sun, 3.0 );
    
    return vec4( col, 1.0 );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (-iResolution.xy + 2.0*fragCoord.xy)/ iResolution.y;
    
    vec2 m = iMouse.xy/iResolution.xy;
    
    // camera
    vec3 ro = 4.0*normalize(vec3(sin(3.0*m.x), 0.4*m.y, cos(3.0*m.x)));
	vec3 ta = vec3(0.0, -1.0, 0.0);
    mat3 ca = setCamera( ro, ta, 0.0 );
    // ray
    vec3 rd = ca * normalize( vec3(p.xy,1.5));
    
    fragColor = render( ro, rd );
}

void mainVR( out vec4 fragColor, in vec2 fragCoord, in vec3 fragRayOri, in vec3 fragRayDir )
{
    fragColor = render( fragRayOri, fragRayDir );
}






