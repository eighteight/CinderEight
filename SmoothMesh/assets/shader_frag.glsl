#version 110

uniform sampler2DShadow	tex0;	// shadow depth texture
uniform sampler2D		tex1;	// diffuse color map

uniform bool flat;

varying vec3 V;
varying vec3 N;
varying vec4 Q;

void main()
{	
	const vec4  gamma = vec4(1.0 / 2.2); 
	const float shinyness = 50.0;

	vec4 ambient = gl_LightSource[0].ambient;
	vec4 diffuse = gl_LightSource[0].diffuse;
	vec4 specular = gl_LightSource[0].specular;

	// if flat shading is enabled, calculate face normal on-the-fly
	vec3 n = flat ? normalize(cross(dFdx(V), dFdy(V))) : N;

	vec3 L = normalize(gl_LightSource[0].position.xyz - V);
	vec3 E = normalize(-V);
	vec3 R = normalize(-reflect(L,n));
	
	// shadow term
	float shadow = 0.1 + 0.9 * shadow2D( tex0, 0.5 * (Q.xyz / Q.w + 1.0) ).r;

	// ambient term 
	vec4 Iamb = ambient;

	// diffuse term
	vec4 Idiff = texture2D( tex1, gl_TexCoord[0].st) * diffuse;
	Idiff *= max(dot(n,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0) * shadow;

	// specular term
	vec4 Ispec = specular;
	Ispec *= pow(max(dot(R,E),0.0), shinyness);
	Ispec = clamp(Ispec, 0.0, 1.0) * shadow;

	// final color 
	gl_FragColor = pow(Iamb + Idiff + Ispec, gamma);
}