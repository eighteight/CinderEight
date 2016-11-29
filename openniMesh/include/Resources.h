#pragma once
#include "cinder/CinderResources.h"

#define RES_VERT_ID				CINDER_RESOURCE( ../resources/, mainVert.glsl,	128, GLSL )
#define RES_FRAG_ID				CINDER_RESOURCE( ../resources/, mainFrag.glsl,	129, GLSL )
#define RES_SHADER_USER_FRAG CINDER_RESOURCE( ../resources/, userFrag.glsl, 130, GLSL )
#define RES_SHADER_USER_VERT CINDER_RESOURCE( ../resources/, userVert.glsl, 131, GLSL )