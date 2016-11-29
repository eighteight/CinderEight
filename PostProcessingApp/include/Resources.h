#pragma once
#include "cinder/CinderResources.h"

#define RES_ICON_APP		CINDER_RESOURCE( ../resources/, app.ico, 128, ICON )

#define RES_TEXTURE			CINDER_RESOURCE( ../resources/, test.png, 130, IMAGE )

#define RES_POSTPROCESS_VERT	CINDER_RESOURCE( ../resources/, post_process_vert.glsl, 140, GLSL )
#define RES_POSTPROCESS_FRAG	CINDER_RESOURCE( ../resources/, post_process_frag.glsl, 141, GLSL )
