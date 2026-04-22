#include "resources.hpp"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

namespace clayborne {
	clayborne::resources init_resources(SDL_Renderer *renderer) {
		auto surface{ IMG_Load("data/temp.png") };
		if (!surface) {
			SDL_Log("Could not load image: %s", SDL_GetError());
		}

		auto spritesheet{ SDL_CreateTextureFromSurface(renderer, surface) };	
		if (!spritesheet) {
			SDL_Log("Could not create texture from surface: %s", SDL_GetError());
		}

		clayborne::resources resources{
			.spritesheet = spritesheet,
		};

		return resources;
	}
}