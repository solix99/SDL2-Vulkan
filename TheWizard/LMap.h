#pragma once

#include "Ltexture.h"
#include <SDL.h>
#include <fstream>

#define MAX_MAP_TEXTURES 99
#define MAX_MAP_OBJECTS 9999

class LMap
{
public:
	LMap();

	void initMap(SDL_Renderer* gRenderer, string texturePackPath, int texturePackSize, string dataPath, int logicalX, int logicalY, int resoutionX, int resolutionY,int offsetX,int offsetY);
	SDL_Point getMapSize();
	SDL_Point getTexturePos(int textureID);
	void renderMap(SDL_Renderer* gRenderer,SDL_Point playerPos);
	void setMapBackground(LTexture &pTexture);
	LTexture *getBackgroundTexture();
	SDL_Rect getCamera();
	void setCameraOffset(int offsetX, int offsetY);
	void insertObject(int OBJECT_ID, int posX, int posY);
	LTexture* getObjectTexture(int i);
	string *getDataPath();
	void deleteObject(SDL_Rect targetRect);
	bool checkCollision(SDL_Rect a, SDL_Rect b);
	void updateMapData();
	bool getTextureCollisionBool(int i);
	bool getObjectSlotUsed(int i);
	SDL_Rect getTextureCollisionRect(int i);
	int getMaxObjects();
	int getObjectTextureID(int i);

private:
	LTexture MAP_BACKGROUND_;
	LTexture MAP_TEXTURE_[MAX_MAP_TEXTURES];
	SDL_Rect TEXTURE_RECT_[MAX_MAP_TEXTURES];
	SDL_Rect OBJECT_RECT_[MAX_MAP_OBJECTS];
	unsigned int TEXTURE_ID_[MAX_MAP_OBJECTS];
	bool TEXTURE_SLOT_USED_[MAX_MAP_OBJECTS];
	bool TEXTURE_HAS_COLLISIION_[MAX_MAP_OBJECTS];
	string DATA_PATH_;

	fstream MAP_STREAM_;
	SDL_Point SCREEN_RESOLUTION_;
	SDL_Point MAP_SIZE_;
	SDL_Point cameraOffset;
	SDL_Rect CLIP_RECT_;
	SDL_Renderer* gRenderer_;
	SDL_Rect camera;
};

