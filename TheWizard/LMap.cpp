#include "LMap.h"

LMap::LMap()
{

}

void LMap::initMap(SDL_Renderer * gRenderer, int logicalX, int logicalY, int resolutionX, int resolutionY, LTexture textureBackground)
{
	MAP_SIZE_.x = logicalX;
	MAP_SIZE_.y = logicalY;
	
	SCREEN_RESOLUTION_.x = resolutionX;
	SCREEN_RESOLUTION_.y = resolutionY;

	MAP_BACKGROUND_ = textureBackground;

	gRenderer_ = gRenderer;

}

void LMap::renderMap(SDL_Point playerPos)
{
	//MAP_BACKGROUND_.render(gRenderer_,0,0)
}