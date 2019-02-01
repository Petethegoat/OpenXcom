/*
 * An implementation of a renderer using the SDL2 renderer infrastructure
 */
#ifndef OPENXCOM_SDLRENDERER_H
#define OPENXCOM_SDLRENDERER_H

#include <SDL.h>
#include "Renderer.h"
#include <string>

namespace OpenXcom
{

class SDLRenderer :
	public Renderer
{
private:
	static const RendererType _rendererType = RENDERER_SDL2;
	SDL_Window *_window;
	SDL_Renderer *_renderer;
	SDL_Texture *_texture;
	SDL_Rect _srcRect, _dstRect;
	Uint32 _format;
	std::string _scaleHint;
public:
	SDLRenderer(SDL_Window *window, int driver, Uint32 flags);
	~SDLRenderer(void);
	void setPixelFormat(Uint32 format);
	void setInternalRect(SDL_Rect *srcRect);
	/// Sets the desired output rectangle.
	void setOutputRect(SDL_Rect *dstRect);
	/// Blits the contents of the SDL_Surface to the screen.
	void flip(SDL_Surface *srcSurface);
	/// INTERNAL: List available renderer drivers
	void listSDLRendererDrivers();
	void screenshot(const std::string &filename) const;
	RendererType getRendererType() { return _rendererType; };
};

}
#endif