#include "LMap.h"

LMap::LMap()
{

}

void LMap::initMap(SDL_Renderer * gRenderer, int logicalX, int logicalY, int resolutionW, int resolutionH)
{
	MAP_SIZE_.x = logicalX;
	MAP_SIZE_.y = logicalY;
	
	SCREEN_RESOLUTION_.x = resolutionW;
	SCREEN_RESOLUTION_.y = resolutionH;

	gRenderer_ = gRenderer;

	CLIP_RECT_.x = 0;
	CLIP_RECT_.y = 0;
	CLIP_RECT_.w = resolutionW;
	CLIP_RECT_.h = resolutionH;
}

void LMap::renderMap(SDL_Renderer * gRenderer)
{
	MAP_BACKGROUND_.render(gRenderer, 0, 0, &CLIP_RECT_, NULL, NULL, SDL_FLIP_NONE, 0, 0, 0, 0, 0);
}

void LMap::setMapBackground(LTexture &pTexture)
{
	MAP_BACKGROUND_ = pTexture;
}
