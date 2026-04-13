#ifndef CLAYBORNE_RESOURCES_HPP
#define CLAYBORNE_RESOURCES_HPP

#include <SDL3/SDL.h>

namespace clayborne {
	struct resources {
		SDL_Texture *spritesheet; // Might want an array for multiple "pages" of sprites
	};

	clayborne::resources init_resources(SDL_Renderer *renderer);
}

#endif // CLAYBORNE_RESOURCES_HPP