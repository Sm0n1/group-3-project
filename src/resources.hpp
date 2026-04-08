#ifndef CLAYBORNE_RESOURCES_HPP
#define CLAYBORNE_RESOURCES_HPP

#include <SDL3/SDL.h>

namespace clayborne {
	struct resources {
		SDL_Renderer *renderer; // Could be a problem if the renderer is deconstructed for some reason?
		SDL_Texture *spritesheet; // Might want an array for multiple "pages" of sprites
	};

	clayborne::resources init_resources(SDL_Renderer *renderer);
}

#endif // CLAYBORNE_RESOURCES_HPP