#include "LPawn.h"


LPawn::LPawn()
{
	mCollider.x = 0;
	mCollider.y = 0;
	mCollider.w = 0;
	mCollider.h = 0;
	xCollisionOffset = 0;
	yCollisionOffset = 0;

	pDefaultHealth = 100;
	p_health = pDefaultHealth;

	isDead = true;

}
//
void LPawn::handleEvent(SDL_Event& e,SDL_Point mapSize)
{
	moving = false;
	if (currentKeyStates[SDL_SCANCODE_W] && mCollider.y > 1)
	{
		mCollider.y -= DEFAULT_VEL;
		moving = true;
	}
	if (currentKeyStates[SDL_SCANCODE_S] && mCollider.y < mapSize.x && (mCollider.y + 0 < mapSize.y-100))
	{
		mCollider.y += DEFAULT_VEL;
		moving = true;
	}
	if (currentKeyStates[SDL_SCANCODE_A] && mCollider.x > 1)
	{
		mCollider.x -= DEFAULT_VEL;
		charDir = true;
		moving = true;
	}
	if (currentKeyStates[SDL_SCANCODE_D] && mCollider.y < mapSize.y && (mCollider.x + 0 < mapSize.x-100))
	{
		mCollider.x += DEFAULT_VEL;
		charDir = false;
		moving = true;
	}

	p_collisionRect.x = mCollider.x + xCollisionOffset;
	p_collisionRect.y = mCollider.y + yCollisionOffset;


	if ((mCollider.x < 0) || (mCollider.x + 0 > mapSize.x))
	{
		//Move back
		//mCollider.x -= DEFAULT_VEL;
	}

	if ((mCollider.y < 0) || (mCollider.y + 0 > mapSize.y))
	{
		//Move back
		//mCollider.y -= DEFAULT_VEL;
	}

}

//void LPawn::move()
//{
//	mCollider.x += mVelX;
//
//	//If the dot went too far to the left or right
//	if ((mCollider.x < 0) || (mCollider.x + TEXTURE_WIDTH > 1280))
//	{
//		//Move back
//		mCollider.x -= mVelX;
//	}
//	//Move the dot up or down
//	mCollider.y += mVelY;
//
//	//If the dot went too far up or down
//	if ((mCollider.y < 0) || (mCollider.y + TEXTURE_HEIGHT > 720))
//	{
//		//Move back
//		mCollider.y -= mVelY;
//	}
//
//}

bool LPawn::checkCollision(SDL_Rect a, SDL_Rect b)
{
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

bool LPawn::getIfSlotUsed()
{
	return slotUsed;
}
void LPawn::setIfSlotUsed(bool b)
{
	slotUsed = b;
}
int LPawn::getPosX()
{
	return mCollider.x;
}
int LPawn::getPosY()
{
	return mCollider.y;
}
int LPawn::getVelX()
{
	return mVelX;
}
int LPawn::getVelY()
{
	return mVelY;
}
int LPawn::getCharDir()
{
	return charDir;
}
SDL_Rect LPawn::getmCollider()
{
	return mCollider;
}
void LPawn::setPosX(int posX)
{
	mCollider.x = posX;
	p_collisionRect.x = mCollider.x + xCollisionOffset;

}
void LPawn::setCollisionOffset(int x, int y)
{
	xCollisionOffset = x;
	yCollisionOffset = y;
}
void LPawn::setPosY(int posY)
{
	mCollider.y = posY;
	p_collisionRect.y = mCollider.y + yCollisionOffset;

}
void LPawn::damageTarget(int d)
{
	p_health -= d;
}

void LPawn::setMCWH(int W, int H)
{
	mCollider.w = W;
	mCollider.h = H;
}
void LPawn::setTexture(LTexture& gTexture)
{
	mPawnTexture = gTexture;
}
LTexture LPawn::getTexture()
{
	return mPawnTexture;
}
void LPawn::setCharDIR(bool b)
{
	charDir = b;
}
bool LPawn::getIfMoving()
{
	return moving;
}
void LPawn::spawnProjectile(int x, int y, int ang, int dx, int dy, float projSpeed)
{
	for (unsigned int i = 0; i < 30; i++)
	{
		if (gProjectile[i].getSlotFree())
		{
			gProjectile[i].setDestX(dx);
			gProjectile[i].setDestY(dy);
			gProjectile[i].setSlotFree(false);
			gProjectile[i].setPosX(x);
			gProjectile[i].setPosY(y);
			gProjectile[i].setAngle(90 + (atan2(dy - y, dx - x) * 180 / 3.14f));
			gProjectile[i].setVelX((dx - x) / projSpeed);
			gProjectile[i].setVelY((dy - y) / projSpeed);

			break;
		}
	}
}

bool LPawn::getProjectileActive()
{
	return projectileActive;
}
void LPawn::setProjectileActive(bool b)
{
	projectileActive = b;
}
void LPawn::setPlayerID(string ID)
{
	playerID = ID;
}
string LPawn::getPlayerID()
{
	return playerID;
}
string LPawn::getNickname()
{
	return nickname;
}
void LPawn::setNickname(string n)
{
	nickname = n;
}
string LPawn::getFlipTypeString()
{
	return flipTypeString;
}
string LPawn::getAnimType()
{
	return animType;
}
SDL_RendererFlip LPawn::getFlipType()
{
	return flipType;
}

void LPawn::setFlipTypeString(string f)
{
	flipTypeString = f;
	
}
void LPawn::setAnimType(string at)
{
	animType = at;
}

void LPawn::setFlipType(SDL_RendererFlip flip)
{
	flipType = flip;
}
int LPawn::getProjectilesActive()
{
	int k = 0;
	for (int i = 0; i < 30; i++)
	{
		if (!gProjectile[i].getSlotFree())
		{
			k++;
		}
	}
	return k;
}
string LPawn::getID()
{
	return playerID;
}

int LPawn::getHealth()
{
	return p_health;
}
void LPawn::setHealth(int h)
{
	p_health = h;
}
SDL_Rect LPawn::getCollisionRect()
{
	return p_collisionRect;
}
void LPawn::setCollisionRectWH(int W, int H)
{
	p_collisionRect.w = W;
	p_collisionRect.h = H;
}
void LPawn::setPlayerDead(bool b)
{
	isDead = b;
}
bool LPawn::getPlayerDead()
{
	return isDead;
}
void LPawn::resetData()
{
	slotUsed = false;
	mCollider.x = 0;
	mCollider.y = 0;
	p_health = pDefaultHealth;
	isDead = true;
	p_health = 100;
}

SDL_Point LPawn::getPlayerPoint()
{
	playerPoint.x = mCollider.x;
	playerPoint.y = mCollider.y;

	return playerPoint;
}
