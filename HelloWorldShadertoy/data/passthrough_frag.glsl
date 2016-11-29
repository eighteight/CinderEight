#version 110
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform sampler2DRect iChannel0;          // input channel. XX = 2D/Cube
uniform vec3      iMouse;

void main (void){

  vec4 color= texture2DRect( iChannel0, gl_TexCoord[0].st * vec2( iResolution.x, iResolution.y ) );
  gl_FragColor = color;
}
