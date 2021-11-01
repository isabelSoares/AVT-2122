#ifndef __FLARE_H
#define __FLARE_H

/* --- Defines --- */

#define FLARE_MAXELEMENTSPERFLARE         15

/* --- Types --- */

#include "VSShaderlib.h"

typedef struct FLARE_ELEMENT_DEF {
    
    float			fDistance;        // Distance along ray from source (0.0-1.0)
    float           fSize;            // Size relative to flare envelope (0.0-1.0)
	float			matDiffuse[4];  // color
	int				textureId;	
} FLARE_ELEMENT_DEF;

typedef struct FLARE_DEF {
    float           fScale;     // Scale factor for adjusting overall size of flare elements.
    float           fMaxSize;   // Max size of largest element, as proportion of screen width (0.0-1.0)
    int             nPieces;    // Number of elements in use
    FLARE_ELEMENT_DEF    element[FLARE_MAXELEMENTSPERFLARE];
} FLARE_DEF;

void initFlare(char* file_path);
void renderWholeFlare(VSShaderLib& shader, MyVec3 lightPos);
#endif
