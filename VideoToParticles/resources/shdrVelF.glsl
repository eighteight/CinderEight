#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform sampler2D texNoise;
uniform sampler2D texNoise2;

uniform float time;
uniform float speed;
uniform float direction;

varying vec4 texCoord;

float tStep = .01;

void main()
{
    
	vec3 pos =		texture2D( positions,	texCoord.st).rgb;
	float mass =	texture2D( positions,	texCoord.st).a;
	vec3 vel =		texture2D( velocities,	texCoord.st).rgb;
	float decay =	texture2D( velocities,	texCoord.st).a;
	float age =		texture2D( information, texCoord.st).r;
	float maxAge =	texture2D( information, texCoord.st).g;
	
	vec2 coords = vec2(pos.x, 1.0-pos.y);
	vec2 depthNoise = texture2D(texNoise2, coords).rg - 0.5;
	depthNoise.r *= direction;
    vec2 noise = 0.001 * depthNoise;
	vel += vec3(noise.x,noise.y,0.0) * speed;
    
    pos.x += vel.x;
    pos.y += vel.y;
	
	age += tStep;
	
	if( age >= 1.0 )
    {
        vec3 origVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 origPos = texture2D(oPositions, texCoord.st).rgb;
        
        age = 0.0;
        
        if(pos.x > 1.0 || pos.x < 0.0 || pos.y > 1.0 || pos.y < 0.0 )
            pos = origPos;
        vel = origVel;
    }
	
	// we write to gl_FragData[n], n corresponds to the attachment of the bound FBO
	
    //position + mass
	gl_FragData[0] = vec4(pos, mass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, decay);
    //age information
	gl_FragData[2] = vec4(age, maxAge, vel.b, 1.0);
}