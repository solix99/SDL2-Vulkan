#include "LMap.h"

LMap::LMap()
{

}

void LMap::initMap(SDL_Renderer * gRenderer, string texturePackPath, int texturePackSize,string dataPath ,int logicalX, int logicalY, int resolutionW, int resolutionH, int offsetX, int offsetY)
{

	memset(TEXTURE_HAS_COLLISIION_, true, MAX_MAP_OBJECTS * sizeof(*TEXTURE_HAS_COLLISIION_));

	TEXTURE_HAS_COLLISIION_[10] = false;
	TEXTURE_HAS_COLLISIION_[13] = false;
	TEXTURE_HAS_COLLISIION_[6] = false;
	TEXTURE_HAS_COLLISIION_[11] = false;
	TEXTURE_HAS_COLLISIION_[12] = false;
	TEXTURE_HAS_COLLISIION_[13] = false;
	TEXTURE_HAS_COLLISIION_[14] = false;
	TEXTURE_HAS_COLLISIION_[15] = false;
	TEXTURE_HAS_COLLISIION_[16] = false;
	TEXTURE_HAS_COLLISIION_[17] = false;
	TEXTURE_HAS_COLLISIION_[18] = false;
	TEXTURE_HAS_COLLISIION_[19] = false;
	TEXTURE_HAS_COLLISIION_[21] = false;
	TEXTURE_HAS_COLLISIION_[29] = false;
	TEXTURE_HAS_COLLISIION_[30] = false;
	TEXTURE_HAS_COLLISIION_[36] = false;
	TEXTURE_HAS_COLLISIION_[37] = false;
	TEXTURE_HAS_COLLISIION_[38] = false;
	TEXTURE_HAS_COLLISIION_[39] = false;
	TEXTURE_HAS_COLLISIION_[40] = false;
	TEXTURE_HAS_COLLISIION_[41] = false;
	TEXTURE_HAS_COLLISIION_[43] = false;
	TEXTURE_HAS_COLLISIION_[44] = false;
	TEXTURE_HAS_COLLISIION_[30] = false;
	TEXTURE_HAS_COLLISIION_[31] = false;

	DATA_PATH_ = dataPath;

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

	int rTick = strlen(texturePackPath.c_str()) - 5;

	for (unsigned int i = 0; i < texturePackSize; i++)
	{
		if (!MAP_TEXTURE_[i].loadFromFile(texturePackPath, gRenderer))
		{
			cout << endl << "Failed to load " << texturePackPath << " | Tick count:" << texturePackSize;
		}
		if (i < 9)
		{
			texturePackPath.replace(rTick, rTick, to_string(i + 1));
		}
		else
		{
			texturePackPath.replace(rTick - 1, rTick - 1, to_string(i + 1));
		}
		texturePackPath.insert(rTick + 1, ".png");
		TEXTURE_RECT_[i].w = MAP_TEXTURE_[i].getWidth();
		TEXTURE_RECT_[i].h = MAP_TEXTURE_[i].getHeight();
	}

	int data[3], k = 0;

	MAP_STREAM_.open(dataPath, ios::in);

	while (MAP_STREAM_ >> data[0] >> data[1] >> data[2])
	{
		for (int i = 0; i < MAX_MAP_OBJECTS; i++)
		{
			if (!TEXTURE_SLOT_USED_[i])
			{
				TEXTURE_ID_[i] = data[0];
				OBJECT_RECT_[i].x = data[1];
				OBJECT_RECT_[i].y = data[2];
				OBJECT_RECT_[i].w = TEXTURE_RECT_[TEXTURE_ID_[i]].w;
				OBJECT_RECT_[i].h = TEXTURE_RECT_[TEXTURE_ID_[i]].h;
				TEXTURE_SLOT_USED_[i] = true;
				k = i;
				break;
			}
		}
	}

	cout << endl << "MAP OBJECTS:" << k;

	MAP_STREAM_.close();
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
	
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (TEXTURE_SLOT_USED_[i])
		{
			MAP_TEXTURE_[TEXTURE_ID_[i]].renderSimple(gRenderer, OBJECT_RECT_[i].x - camera.x, OBJECT_RECT_[i].y - camera.y);
		}
	}
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
void LMap::insertObject(int OBJECT_ID,int posX, int posY)
{
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (!TEXTURE_SLOT_USED_[i])
		{
			TEXTURE_ID_[i] = OBJECT_ID;
			OBJECT_RECT_[i].x = posX;
			OBJECT_RECT_[i].y = posY;
			OBJECT_RECT_[i].w = TEXTURE_RECT_[TEXTURE_ID_[i]].w;
			OBJECT_RECT_[i].h = TEXTURE_RECT_[TEXTURE_ID_[i]].h;
			TEXTURE_SLOT_USED_[i] = true;
			break;
		}
	}
}

LTexture* LMap::getObjectTexture(int i)
{
	return &MAP_TEXTURE_[i];
}
string *LMap::getDataPath()
{
	return &DATA_PATH_;
}

void LMap::deleteObject(SDL_Rect targetRect)
{
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (TEXTURE_SLOT_USED_[i])
		{
			if (checkCollision(targetRect, OBJECT_RECT_[i]))
			{
				TEXTURE_SLOT_USED_[i] = false;
				updateMapData();
				break;
			}
		}
	}
}

bool LMap::checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

void LMap::updateMapData()
{
	MAP_STREAM_.open(DATA_PATH_, ios::out);

	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (TEXTURE_SLOT_USED_[i])
		{
			MAP_STREAM_ << TEXTURE_ID_[i] << " " << OBJECT_RECT_[i].x << " " << OBJECT_RECT_[i].y << " ";
		}
	}

	MAP_STREAM_.close();
}

bool LMap::getTextureCollisionBool(int i)
{
	return TEXTURE_HAS_COLLISIION_[TEXTURE_ID_[i]];
}

bool LMap::getObjectSlotUsed(int i)
{
	return TEXTURE_SLOT_USED_[i];
}

SDL_Rect LMap::getTextureCollisionRect(int i) 
{
	SDL_Rect croppedRect = OBJECT_RECT_[i];
	croppedRect.x = croppedRect.x + croppedRect.w * 0.2f;
	croppedRect.y = croppedRect.y + croppedRect.h * 0.2f;
	croppedRect.w = croppedRect.w * 0.6f;
	croppedRect.h = croppedRect.h * 0.6f;

	return croppedRect;
}

int LMap::getMaxObjects()
{
	return MAX_MAP_OBJECTS;
}
int LMap::getObjectTextureID(int i)
{
	return TEXTURE_ID_[i];
}