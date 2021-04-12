#include "Ltexture.h"

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;

}
////

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

SDL_Texture *LTexture::getTexture()
{
	return mTexture;
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(SDL_Renderer* gRenderer, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip, bool renderCollisionBox, int colW, int colH, int colX, int colY)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	if (renderCollisionBox)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

		collisionBox.x = colX;
		collisionBox.y = colY;
		collisionBox.w = colW;
		collisionBox.h = colH;
		SDL_RenderDrawRect(gRenderer, &collisionBox);
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}
void LTexture::renderSimple(SDL_Renderer* gRenderer, int x, int y)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(gRenderer, &collisionBox);

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture , NULL, &renderQuad, 0,0, SDL_FLIP_NONE);
}


int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}


LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* gRenderer, TTF_Font* gFont)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			//cout<<"Unable to create texture from rendered text! SDL Error:\n" << SDL_GetError();
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
			collisionBox.w = mWidth;
			collisionBox.h = mHeight;

		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		//cout<<"Unable to render text surface! SDL_ttf Error: \n" << TTF_GetError();
	}


	//Return success
	return mTexture != NULL;
}

bool LTexture::loadFromFile(std::string path, SDL_Renderer* gRenderer)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		cout << "Unable to load image SDL Error:\n" << IMG_GetError();
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 25, 255));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			cout << "Unable to create texture SDL Error:\n" << SDL_GetError();
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
			collisionBox.w = mWidth;
			collisionBox.h = mHeight;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::setRenderTarget(SDL_Renderer* gRenderer)
{
	SDL_SetRenderTarget(gRenderer, mTexture);

	return true;
}

bool LTexture::loadTargetTexture(SDL_Renderer* gRenderer,int w,int h)
{
	mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
	SDL_SetRenderTarget(gRenderer,mTexture);

	return true;
}
void LTexture::setTexture(SDL_Texture* Texture)
{
	mTexture = Texture;
}
