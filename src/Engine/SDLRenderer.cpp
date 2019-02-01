


#include "SDLRenderer.h"
#include <assert.h>
#include "Exception.h"
#include "Logger.h"
#include "../lodepng.h"

namespace OpenXcom
{

SDLRenderer::SDLRenderer(SDL_Window *window, int driver, Uint32 flags): _window(window), _texture(NULL), _renderer(NULL), _format(SDL_PIXELFORMAT_ARGB8888)
{
	listSDLRendererDrivers();
	_renderer = SDL_CreateRenderer(window, -1, flags);
	_srcRect.x = _srcRect.y = _srcRect.w = _srcRect.h = 0;
	_dstRect.x = _dstRect.y = _dstRect.w = _dstRect.h = 0;
	if (_renderer == NULL)
	{
		Log(LOG_ERROR) << "[SDLRenderer] Couldn't create renderer; error message: " << SDL_GetError();
		throw Exception(SDL_GetError());
	}
	Log(LOG_INFO) << "[SDLRenderer] Renderer created";
	const char *scaleHint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
	if (!scaleHint)
	{
		_scaleHint = "nearest";
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _scaleHint.c_str());
	}
	else
	{
		_scaleHint = scaleHint;
	}
	SDL_RendererInfo info;
	SDL_GetRendererInfo(_renderer, &info);
	Log(LOG_INFO) << "[SDLRenderer] Created new SDLRenderer, using " << info.name;
	Log(LOG_INFO) << "[SDLRenderer] Current scaler is " << _scaleHint;
}

void SDLRenderer::setPixelFormat(Uint32 format)
{
	_format = format;
	Log(LOG_INFO) << "[SDLRenderer] Texture pixel format set to " << SDL_GetPixelFormatName(_format);
}

void SDLRenderer::setInternalRect(SDL_Rect *srcRect)
{
	// Internal rectangle should not have any X or Y offset.
	assert(srcRect->x == 0 && srcRect->y == 0);
	if (!_texture)
	{
		_texture = SDL_CreateTexture(_renderer,
						_format,
						SDL_TEXTUREACCESS_STREAMING,
						srcRect->w,
						srcRect->h);
	}
	else
	{
		int w, h, access;
		Uint32 format;
		const char *scaleHint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
		SDL_QueryTexture(_texture, &format, &access, &w, &h);
		Log(LOG_INFO) << "[SDLRenderer] Old scale hint: " << _scaleHint << ", new scale hint: " << scaleHint;
		if ( (w != srcRect->w) || (h != srcRect->h)
			|| (_scaleHint != scaleHint) )
		{
			SDL_DestroyTexture(_texture);
			_texture = SDL_CreateTexture(_renderer,
							format,
							access,
							srcRect->w,
							srcRect->h);
			_scaleHint = scaleHint;
		}
	}
	if (_texture == NULL)
	{
		throw Exception(SDL_GetError());
	}
	_srcRect.w = srcRect->w;
	_srcRect.h = srcRect->h;
	Log(LOG_INFO) << "[SDLRenderer] Texture resolution set to " << _srcRect.w << "x" << _srcRect.h;
}

void SDLRenderer::setOutputRect(SDL_Rect *dstRect)
{
	_dstRect.x = dstRect->x;
	_dstRect.y = dstRect->y;
	_dstRect.w = dstRect->w;
	_dstRect.h = dstRect->h;
	Log(LOG_INFO) << "[SDLRenderer] Output resolution: " << _dstRect.w << "x" << _dstRect.h;
	Log(LOG_INFO) << "[SDLRenderer] Offset: " << _dstRect.x << "x" << _dstRect.y;
}


SDLRenderer::~SDLRenderer(void)
{
	SDL_DestroyTexture(_texture);
	SDL_DestroyRenderer(_renderer);
}

void SDLRenderer::flip(SDL_Surface *srcSurface)
{
	SDL_UpdateTexture(_texture, &_srcRect, srcSurface->pixels,
			srcSurface->pitch);
	SDL_RenderClear(_renderer);
	SDL_RenderCopy(_renderer, _texture, &_srcRect, &_dstRect);
	SDL_RenderPresent(_renderer); //TODO: check error?
}

void SDLRenderer::listSDLRendererDrivers()
{
	int numRenderDrivers = SDL_GetNumRenderDrivers();
	Log(LOG_INFO) << "[SDLRenderer] Listing available rendering drivers:";
	Log(LOG_INFO) << "[SDLRenderer]  Number of drivers: " << numRenderDrivers;
	for (int i = 0; i < numRenderDrivers; ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		Log(LOG_INFO) << "[SDLRenderer]  Driver " << i << ": " << info.name;
		Log(LOG_INFO) << "[SDLRenderer]    Number of texture formats: " << info.num_texture_formats;
		for (Uint32 j = 0; j < info.num_texture_formats; ++j)
		{
			Log(LOG_INFO) << "[SDLRenderer]     Texture format " << j << ": " << SDL_GetPixelFormatName(info.texture_formats[j]);
		}
	}
}

void SDLRenderer::screenshot(const std::string &filename) const
{
	int width, height;
	SDL_GetWindowSize(_window, &width, &height);
	unsigned char *pixels = new unsigned char[width * height * 4];
	unsigned error = SDL_RenderReadPixels(_renderer, NULL, SDL_PIXELFORMAT_ABGR8888, (void*)pixels, width * 4);
	if (error)
	{
		Log(LOG_ERROR) << "Acquiring pixels failed while trying to save screenshot: " << SDL_GetError();
		delete[] pixels;
		return;
	}
	error = lodepng::encode(filename, (const unsigned char*) pixels, width, height, LCT_RGBA);
	if (error)
	{
		Log(LOG_ERROR) << "Saving to PNG failed: " << lodepng_error_text(error);
	}
	delete[] pixels;
}

}
