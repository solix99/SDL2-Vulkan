#include "LMap.h"

LMap::LMap()
{

}

void LMap::initMap(SDL_Renderer * gRenderer, int logicalX, int logicalY, int resolutionW, int resolutionH, int offsetX, int offsetY)
{
	MAP_SIZE_.x = logicalX;
	MAP_SIZE_.y = logicalY;
	
	SCREEN_RESOLUTION_.x = resolutionW;
	SCREEN_RESOLUTION_.y = resolutionH;

	gRenderer_ = gRenderer;

	CLIP_RECT_.x = 0;
	CLIP_RECT_.y = 0;
	CLIP_RECT_.w = logicalX;
	CLIP_RECT_.h = logicalY;

	cameraOffset.x = offsetX;
	cameraOffset.y = offsetY;

	camera = { 0, 0,  SCREEN_RESOLUTION_.x, SCREEN_RESOLUTION_.y };
}

void LMap::renderMap(SDL_Renderer * gRenderer, SDL_Point playerPos)
{
	camera.x = (playerPos.x + cameraOffset.x /2) - (SCREEN_RESOLUTION_.x / 2);
	camera.y = (playerPos.y + cameraOffset.y /2) - (SCREEN_RESOLUTION_.y / 2);

	if (camera.x < 0)
	{
		camera.x = 0;
	}
	if (camera.y < 0)
	{
		camera.y = 0;
	}
	if (camera.x > MAP_SIZE_.x - camera.w)
	{
		camera.x = MAP_SIZE_.x - camera.w;
	}
	if (camera.y > MAP_SIZE_.y - camera.h)
	{
		camera.y = MAP_SIZE_.y - camera.h;
	}

	MAP_BACKGROUND_.render(gRenderer, 0, 0, &camera, NULL, NULL, SDL_FLIP_NONE, 0, 0, 0, 0, 0);

}

void LMap::setMapBackground(LTexture &pTexture)
{
	MAP_BACKGROUND_ = pTexture;
}
LTexture *LMap::getBackgroundTexture()
{
	return &MAP_BACKGROUND_;
}

SDL_Point LMap::getMapSize()
{
	return MAP_SIZE_;
}
SDL_Rect LMap::getCamera()
{
	return camera;
}
void LMap::setCameraOffset(int offsetX, int offsetY)
{
	cameraOffset.x = offsetX;
	cameraOffset.y = offsetY;
}