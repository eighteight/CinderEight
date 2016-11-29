//#extension GL_ARB_draw_buffers : enable
uniform sampler2D posArray;
uniform sampler2D velArray;
uniform float volume;
varying vec4 texCoord;
varying vec4 clr;

void main(void)
{	
	float mass	= texture2D( posArray, texCoord.st).a;
	vec3 p		= texture2D( posArray, texCoord.st).rgb;
	vec3 v		= texture2D( velArray, texCoord.st).rgb;
	vec3 acc	= -0.0002*p*volume; // Centripetal force
	vec3 ayAcc  = 0.00001*normalize(cross(vec3(0, 1 ,0),p)); // Angular force
    vec3 centerSpringAcc = 0.00000000001*(p-vec3(100,100,0))*(p-vec3(100,100,0))*normalize(p-vec3(100,100,0));

	vec3 new_v  = v + mass*(acc+ayAcc+centerSpringAcc);	
	vec3 new_p	= p + new_v;


	//Render to positions texture
	gl_FragData[0] = vec4(new_p.x, new_p.y, new_p.z, mass);
	//Render to velocities texture
	gl_FragData[1] = vec4(new_v.x, new_v.y, new_v.z, 1.0);

}