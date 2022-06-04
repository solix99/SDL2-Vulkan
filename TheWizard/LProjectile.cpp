#include "LProjectile.h"

LProjectile::LProjectile()
{
	mCollider.x = 0;
	mCollider.y = 0;
	mCollider.w = 0;
	mCollider.h = 0;

	xCollisionOffset = 0;
	yCollisionOffset = 0;

	p_DMG = 30;

	mVelX = 5;
	mVelY = 5;
	
	slotFree = true;
}
SDL_Rect LProjectile::getmCollider()
{
	return mCollider;
}
SDL_Rect LProjectile::getCollisionRect()
{
	return p_collisionRect;
}
void LProjectile::setCollisionRectWH(int W,int H)
{
	p_collisionRect.w = W;
	p_collisionRect.h = H;
}
float LProjectile::getXCollisionOffset()
{
	return xCollisionOffset;
}
float LProjectile::getYCollisionOffset()
{
	return yCollisionOffset;
}
void LProjectile::setPosX(float posX)
{
	mPosX = posX;
	mCollider.x = posX;
	p_collisionRect.x = posX + xCollisionOffset;
}
void LProjectile::setPosY(float posY)
{
	mPosY = posY;
	mCollider.y = posY;
	p_collisionRect.y = posY + yCollisionOffset;
}
void LProjectile::setCollisionOffset(float x, float y)
{
	xCollisionOffset = x;
	yCollisionOffset = y;
}
void LProjectile::setMCWH(int W, int H)
{
	mCollider.w = W;
	mCollider.h = H;
}
float LProjectile::getPosX()
{
	return mPosX;
}
float LProjectile::getPosY()
{
	return mPosY;
}
int LProjectile::getDMG()
{
	return p_DMG;
}
void LProjectile::setDMG(int d)
{
	p_DMG = d;
}

bool LProjectile::checkCollision(SDL_Rect FIRST_RECT, SDL_Rect SECOND_RECT)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = FIRST_RECT.x;
	rightA = FIRST_RECT.x + FIRST_RECT.w;
	topA = FIRST_RECT.y;
	bottomA = FIRST_RECT.y + FIRST_RECT.h;

	leftB = SECOND_RECT.x;
	rightB = SECOND_RECT.x + SECOND_RECT.w;
	topB = SECOND_RECT.y;
	bottomB = SECOND_RECT.y + SECOND_RECT.h;

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
	return true;
}
 
void LProjectile::setAngle(float a)
{
	angle = a;
}
float LProjectile::getAngle()
{
	return angle;
}
bool LProjectile::checkStatus()
{
	mPosX += mVelX;
	mPosY += mVelY;

	mCollider.x += mVelX;
	mCollider.y += mVelY;

	p_collisionRect.x = mCollider.x + xCollisionOffset;
	p_collisionRect.y = mCollider.y + yCollisionOffset;


	return true;
}
bool LProjectile::getSlotFree()
{
	return slotFree;
}
void LProjectile::setSlotFree(bool b)
{
	slotFree = b;

	if (b)
	{
		xCollisionOffset = 0;
		yCollisionOffset = 0;
	}

}
int LProjectile::getDestX()
{
	return mDestX;
}
int LProjectile::getDestY()
{
	return mDestY;
}
void LProjectile::setDestX(int x)
{
	mDestX = x;
}
void LProjectile::setDestY(int y)
{
	mDestY = y;
}
float LProjectile::getVelX()
{
	return mVelX;
}
float LProjectile::getVelY()
{
	return mVelY;
}
void LProjectile::setVelX(float x)
{
	mVelX = x;
}
void LProjectile::setVelY(float y)
{
	mVelY = y;
}
float LProjectile::getProjSpeed()
{
	return PROJ_SPEED_;
}
void LProjectile::setProjSpeed(float pSpeed)
{
	PROJ_SPEED_ = pSpeed;
}