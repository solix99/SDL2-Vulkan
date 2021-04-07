#pragma once

#include "Ltexture.h"
#include <SDL.h>

#define MAX_MAP_TEXTURES 999

class LMap
{
public:
	LMap();

	void initMap(SDL_Renderer* gRenderer, int logicalX, int logicalY, int resoutionX, int resolutionY,LTexture textureBackground);
	SDL_Point getMapSize();
	SDL_Point getTexturePos(int textureID);
	void renderMap(SDL_Point playerPos);

private:
	LTexture MAP_BACKGROUND_;
	LTexture MAP_TEXTURE_[MAX_MAP_TEXTURES];
	unsigned int TEXTURE_ID_[MAX_MAP_TEXTURES];
	SDL_Point TEXTURE_POS_[MAX_MAP_TEXTURES];

	SDL_Point SCREEN_RESOLUTION_;
	SDL_Point MAP_SIZE_;

	SDL_Renderer* gRenderer_;

};

