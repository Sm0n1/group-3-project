#include "resources.hpp"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

namespace clayborne {
	clayborne::resources init_resources(SDL_Renderer* renderer) {
		clayborne::resources resources{
			.renderer = renderer,
			.spritesheet = SDL_CreateTextureFromSurface(renderer, IMG_Load("Character A.png"))
		};

		return resources;
	}
}