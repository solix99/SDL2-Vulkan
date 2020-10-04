
#include "LAnim.h"


LAnim::LAnim()
{
	memset(currentTickClient, 0, MAX_VARIABLE_AMMOUNT * sizeof(*currentTickClient));
	for (unsigned int i = 0; i < MAX_VARIABLE_AMMOUNT; i++)
	{
		cropInUse[i] = false;
		isInverse[i] = false;
		for (unsigned j = 0; j < MAX_VARIABLE_AMMOUNT; j++)
		{
			animTimer[i][j].start();
			seqInUse[i][j] = false;
			isInverseSeq[i][j] = false;
		}
	}
	inUse = false;

	tickTime = 1000;
}

bool LAnim::loadAnim(SDL_Renderer* aRenderer, string file,int tk)
{
	tickCount = tk;
	int rTick = strlen(file.c_str()) - 5;

	for (unsigned int i = 0; i < tickCount; i++)
	{
		if (!animTexture[i].loadFromFile(file, aRenderer))
		{
			cout << endl << "Failed to load " << file << " | Tick count:" << tickCount;
			return false;
		}
		if (i < 9)
		{
			file.replace(rTick, rTick, to_string(i + 1));
		}
		else
		{
			file.replace(rTick - 1, rTick - 1, to_string(i + 1));
		}
		file.insert(rTick + 1, ".png");
	}
	
	return true;
}

bool LAnim::loadAnimCrop(SDL_Renderer* a, string file, int rowSize, int columSize, int animSizeX, int animSizeY)
{
	animTexture[0].loadFromFile(file, a);

	int k = 0;

	for (unsigned int i = 0; i < rowSize; i++)
	{
		for (unsigned int j = 0; j < columSize; j++)
		{
			animRect[k].x = (animSizeX * j);
			animRect[k].y = (animSizeY * i);
			animRect[k].w = animSizeX;
			animRect[k].h = animSizeY;

			k++;
		}
	}

	tickCount = rowSize * columSize;
	return true;
}
bool LAnim::getInUse()
{
	return inUse;
}
void LAnim::setInUse(bool b)
{
	inUse = b;
}

void LAnim::updateAnim(int i, int j)
{
	if (!isInverseSeq[i][j])
	{
		seqInUse[i][j] = true;

		if (animTimer[i][j].getTicks() > tickTime / tickCount)
		{
			currentTickClient[i][j]++;
			animTimer[i][j].reset();
		}
		if (currentTickClient[i][j] >= tickCount - 1)
		{
			animTimer[i][j].reset();
			currentTickClient[i][j] = 0;
			if (p_renderInverse)
			{
				isInverseSeq[i][j] = true;
			}
			else
			{
				seqInUse[i][j] = false;
			}
		}
	}
	else if (p_renderInverse && isInverseSeq[i][j])
	{
		if (animTimer[i][j].getTicks() > tickTime / tickCount)
		{
			currentTickClient[i][j]++;
			animTimer[i][j].reset();
		}
		if (currentTickClient[i][j] >= tickCount - 1)
		{
			animTimer[i][j].reset();
			currentTickClient[i][j] = 0;
			isInverseSeq[i][j] = false;
			seqInUse[i][j] = false;
		}

	}
}

void LAnim::renderTexture(SDL_Renderer* gRenderer, int x, int y, float ang, int i,int j, bool inverse ,SDL_RendererFlip flip, int renderSpeed,int renderInverseSpeed,bool renderColisionBox, int colW ,int colH,int colX, int colY)
{
	if (isInverseSeq[i][j])
	{
		animTexture[tickCount - currentTickClient[i][j]].render(gRenderer, x, y, NULL, ang, NULL, flip,renderColisionBox,colW,colH,colX,colY);
	}
	else
	{
		animTexture[currentTickClient[i][j]].render(gRenderer, x, y, NULL, ang, NULL, flip,renderColisionBox,colW,colH,colX,colY);
	}
}

bool LAnim::addNewStaticAnim(int posX, int posY, bool isCrop,bool renderInverse)
{
	p_renderInverse = renderInverse;
	p_isCrop = isCrop;

	if (isCrop)
	{
		for (unsigned int i = 0; i < MAX_VARIABLE_AMMOUNT; i++)
		{
			if (!cropInUse[i])
			{
				cropPosX[i] = posX;
				cropPosY[i] = posY;
				cropInUse[i] = true;

				return true;
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < MAX_VARIABLE_AMMOUNT; i++)
		{
			for (unsigned int j = 0; j < MAX_VARIABLE_AMMOUNT; j++)
			{
				if (!seqInUse[i][j])
				{
					seqPosX[i][j] = posX;
					seqPosY[i][j] = posY;
					seqInUse[i][j] = true;

					return true;
				}
			}
		}
	}

	return true;
}

void LAnim::renderStaticAnim(SDL_Renderer * gRenderer,bool renderCollisiionBox, int colW, int colH,int colX, int colY)
{
	//RENDER SEQ ANIM

	if (!p_isCrop)
	{
		for (unsigned int i = 0; i < MAX_VARIABLE_AMMOUNT; i++)
		{
			for (unsigned int j = 0; j < MAX_VARIABLE_AMMOUNT; j++)
			{
				if (seqInUse[i][j] && isInverseSeq[i][j] == false)
				{	
					animTexture[currentTickClient[i][j]].render(gRenderer, seqPosX[i][j], seqPosY[i][j], NULL, animAngle[i][j], NULL, SDL_FLIP_NONE, renderCollisiionBox, colW, colH,colX,colY);

					if (animTimer[i][j].getTicks() > 500 / tickCount)
					{
						currentTickClient[i][j]++;
						animTimer[i][j].reset();
					}
					if (currentTickClient[i][j] >= tickCount - 1)
					{
						currentTickClient[i][j] = 0;
						isInverseSeq[i][j] = true;
					}
				}
				else if (isInverseSeq[i][j] && p_renderInverse)
				{
					animTexture[tickCount - currentTickClient[i][j]].render(gRenderer, seqPosX[i][j], seqPosY[i][j], NULL, animAngle[i][j], NULL, SDL_FLIP_NONE, renderCollisiionBox, colW, colH,colX,colY);

					if (animTimer[i][j].getTicks() > 500 / tickCount)
					{
						currentTickClient[i][j]++;
						animTimer[i][j].reset();
					}
					if (currentTickClient[i][j] >= tickCount - 1)
					{
						currentTickClient[i][j] = 0;
						isInverseSeq[i][j] = false;
						seqInUse[i][j] = false;
					}
				}
			}
		}
	}
	else
	{
		//RENDER CROPPED ANIMS
		for (unsigned int i = 0; i < MAX_VARIABLE_AMMOUNT; i++)
		{
			if (cropInUse[i] && !isInverse[i])
			{
				animTexture[0].render(gRenderer, cropPosX[i], cropPosY[i], &animRect[currentTickClient[0][i]], 0, 0, SDL_FLIP_NONE,renderCollisiionBox, colW, colH,colX,colY);

				if (animTimer[0][i].getTicks() > tickTime / tickCount)
				{
					currentTickClient[0][i]++;
					animTimer[0][i].reset();
				}
				if (currentTickClient[0][i] >= tickCount - 1)
				{
					animTimer[0][i].reset();
					currentTickClient[0][i] = 0;

					if (p_renderInverse)
					{
						isInverse[i] = true;
					}
					else
					{
						cropInUse[i] = false;
					}
				}

			}
			else if (isInverse[i] && p_renderInverse)
			{
				animTexture[0].render(gRenderer, cropPosX[i], cropPosY[i], &animRect[tickCount - currentTickClient[0][i]], 0, 0, SDL_FLIP_NONE,renderCollisiionBox, colW, colH,colX,colY);

				if (animTimer[0][i].getTicks() > tickTime / tickCount)
				{
					currentTickClient[0][i]++;
					animTimer[0][i].reset();
				}
				if (currentTickClient[0][i] >= tickCount - 1)
				{
					animTimer[0][i].reset();
					currentTickClient[0][i] = 0;
					cropInUse[i] = false;
					isInverse[i] = false;
				}
			}
		}
	}

}

bool LAnim::getIsInverse(bool b)
{
	return b;
}
void LAnim::setCropInUse(int i,bool b)
{
	cropInUse[i] = b;
}
bool LAnim::getCropInUse(int i)
{
	return cropInUse[i];
}
void LAnim::setIsInverseSeq(int i, int j, bool b)
{
	isInverseSeq[i][j] = b;
}
void LAnim::startAnimTimer(int i,int j)
{
	animTimer[i][j].start();
}
void LAnim::stopAnimTimer(int i,int j)
{
	animTimer[i][j].stop();
}
void LAnim::resetAnimTimer(int i,int j)
{
	animTimer[i][j].reset();
}
int LAnim::getAnimTimerTicks(int i,int j)
{
	return animTimer[i][j].getTicks();
}
bool LAnim::getIfTimerStarted(int i,int j)
{
	return animTimer[i][j].isStarted();
}
bool LAnim::getSeqInUse(int i, int j)
{
	return seqInUse[i][j];
}
bool LAnim::getIsInverseSeq(int i, int j)
{
	return isInverseSeq[i][j];
}
int LAnim::getCurrentTickClient(int i,int j)
{
	return currentTickClient[i][j];
}
int LAnim::getTickCount()
{
	return tickCount;
}
void LAnim::setCurrentTickClient(int i,int j, int tick)
{
	currentTickClient[i][j] = tick;
}
void LAnim::setCropPosX(int i, int val)
{
	cropPosX[i] = val;
}
void LAnim::setCropPosY(int i, int val)
{
	cropPosY[i] = val;
}

int LAnim::getCropPosX(int i)
{
	return cropPosX[i];
}
int LAnim::getCropPosY(int i)
{
	return cropPosY[i];
}
void LAnim::setTickTime(int i)
{
	tickTime = i;
}
int LAnim::getTickTime()
{
	return tickTime;
}