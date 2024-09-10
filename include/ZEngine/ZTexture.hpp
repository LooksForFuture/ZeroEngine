#ifndef ZENGINE_ZTEXTURE
#define ZENGINE_ZTEXTURE

#include <ZEngine/Rtti.hpp>
#include <ZEngine/ZAsset.hpp>
#include <ZEngine/ZEngine.hpp>

//#define STB_IMAGE_IMPLEMENTATION
//#include <ZEngine/stb_image.h>

#include <SDL_image.h> //texture loading utilities

class ZTexture : public ZAsset {
	RTTI_DECLARE_TYPEINFO(ZTexture, ZAsset);

	SDL_Texture* texture = NULL;

public:
	void loadFromFile(std::string fileContent) {
		ZRef<ZRenderer> renderer = getRenderer();

		SDL_RWops* rwop = SDL_AllocRW();
		const char* data = fileContent.c_str();
		rwop = SDL_RWFromMem((void*)data, fileContent.size());
		SDL_RWseek(rwop, 0, RW_SEEK_SET);

		texture = renderer->loadTexture(rwop, true);
		if (texture == NULL) {
			std::cerr << "Failed to create texture from: \"" << getPath() << "\" SDL_Error: " << SDL_GetError() << '\n';
		}
		
		loadedSuccessfully = true;
	}

	IVec2 getSize() {
		int w, h;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
		return IVec2(w, h);
	}

	void render(IVec4 src, Vec4 dst, float angle) {
		getRenderer()->renderTexture(texture, src, dst, angle);
	}

	void release() {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
};

#endif // !ZENGINE_ZTEXTURE
