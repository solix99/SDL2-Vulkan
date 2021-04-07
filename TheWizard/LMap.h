#pragma once

#include "Ltexture.h"
#include <SDL.h>

#define MAX_MAP_TEXTURES 999

class LMap
{
public:
	LMap(SDL_Point mapSize, SDL_Point screenResolution);

	SDL_Point getMapSize();
	SDL_Point getTexturePos(int textureID);
	void renderMap(SDL_Point playerPos);

private:
	LTexture MAP_TEXTURE_[MAX_MAP_TEXTURES];
	unsigned int TEXTURE_ID_[MAX_MAP_TEXTURES];
	SDL_Point TEXTURE_POS_[MAX_MAP_TEXTURES];

	SDL_Point SCREEN_RESOLUTION_;
	SDL_Point MAP_SIZE_;

};

