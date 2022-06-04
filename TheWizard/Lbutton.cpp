#include "Lbutton.h"
#include "LTexture.h"

LButton::LButton()
{
	mPosition.x = 0;
	mPosition.y = 0;

}

void LButton::setPosition(int x, int y, int w, int h)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.w = w;
	mPosition.h = h;
}

bool LButton::handleClick(SDL_Event e)
{
	bool inside = true;

	if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);

		if (x < mPosition.x)
		{
			inside = false;
		}
		
		else if (x > mPosition.x + mPosition.w)
		{
			inside = false;
		}

		else if (y < mPosition.y)
		{
			inside = false;
		}

		else if (y > mPosition.y + mPosition.h)
		{
			inside = false;
		}

		if (!inside)
		{
			inside = false;
		}
		else
		{
			inside = true;
		}
	}

	return inside;
}

void LButton::render(SDL_Renderer* gRenderer)
{
	//SDL_Rect a = { 0,0 ,TEXTURE_.getWidth(),TEXTURE_.getHeight()};

	TEXTURE_.render(gRenderer, mPosition.x, mPosition.y, NULL, NULL, NULL, SDL_FLIP_NONE, false,0,0,0,0);
}

void LButton::init(string path, SDL_Renderer* gRenderer,int posX,int posY)
{
	TEXTURE_.loadFromFile(path, gRenderer);
	mPosition.x = posX;
	mPosition.y = posY;
	mPosition.w = TEXTURE_.getWidth();
	mPosition.h = TEXTURE_.getHeight();
}