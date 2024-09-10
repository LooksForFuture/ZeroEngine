#ifndef ZENGINE_ZRENDERER
#define ZENGINE_ZRENDERER

#include <ZEngine/ZCore.hpp>

//Using SDL and extensions
#include <SDL.h>
#undef main

#include <SDL_image.h> //texture loading utilities

#include <initializer_list>
#include <vector>
#include <utility>

class ZInputHandler {
    static bool constructed;
    static std::vector<std::pair<std::string, std::vector<SDL_Scancode>>> inputs;
    static std::vector<std::pair<std::string_view, bool>> states;
    static std::vector<std::pair<std::string_view, bool>> oldStates;

public:
    static void add(std::string name, std::initializer_list<SDL_Scancode> codes) {
        std::pair<std::string, std::vector<SDL_Scancode>>* inputPair = nullptr;

        bool found = false;
        for (std::pair<std::string, std::vector<SDL_Scancode>>& p : inputs) {
            if (p.first == name) {
                inputPair = &p;
                found = true;
                break;
            }
        }

        if (!found) {
            inputs.push_back(std::pair<std::string, std::vector<SDL_Scancode>>());
            inputPair = &(inputs[inputs.size()-1]);
            inputPair->first = name;
        }

        size_t size = codes.size();
        for (SDL_Scancode code : codes) {
            inputPair->second.push_back(code);
        }

        //register in the states vector if does not exist
        found = false;
        for (const auto& p : states) {
            if (p.first == inputPair->first) {
                found = true;
                break;
            }
        }

        if (!found) {
            states.push_back(std::pair<std::string_view, bool>());
            std::pair<std::string_view, bool>* statePair = &states[states.size() - 1];
            statePair->first = inputPair->first;
            statePair->second = false;
        }

        //register in the old states vector if does not exist
        found = false;
        for (const auto& p : oldStates) {
            if (p.first == inputPair->first) {
                found = true;
                break;
            }
        }

        if (!found) {
            oldStates.push_back(std::pair<std::string_view, bool>());
            std::pair<std::string_view, bool>* statePair = &oldStates[oldStates.size() - 1];
            statePair->first = inputPair->first;
            statePair->second = false;
        }
    }

    static void update(SDL_Event* e) {
        const Uint8* keyStates = SDL_GetKeyboardState(NULL);
        size_t inputCount = inputs.size();
        for (size_t i = 0; i < inputCount; i++) {
            std::pair<std::string_view, bool>* statePair = &states[i];
            std::pair<std::string_view, bool>* oldStatePair = &oldStates[i];

            oldStatePair->second = statePair->second;

            statePair->second = false;
            std::pair<std::string, std::vector<SDL_Scancode>>* inputPair = &inputs[i];
            for (SDL_Scancode code : inputPair->second) {
                if (keyStates[code]) {
                    statePair->second = true;
                    break;
                }
            }
        }
    }

    static bool isKeyDown(const char* name) {
        size_t inputCount = inputs.size();
        for (size_t i = 0; i < inputCount; i++) {
            std::pair<std::string_view, bool>* statePair = &states[i];
            if (statePair->first == name) return statePair->second;
        }

        return false;
    }

    static bool isKeyPressed(const char* name) {
        size_t inputCount = inputs.size();
        for (size_t i = 0; i < inputCount; i++) {
            std::pair<std::string_view, bool>* statePair = &states[i];
            std::pair<std::string_view, bool>* oldStatePair = &oldStates[i];
            if (statePair->first == name) return (!oldStatePair->second && statePair->second);
        }

        return false;
    }

    static bool isKeyUp(const char* name) {
        size_t inputCount = inputs.size();
        for (size_t i = 0; i < inputCount; i++) {
            std::pair<std::string_view, bool>* statePair = &states[i];
            if (statePair->first == name) return !statePair->second;
        }

        return false;
    }

    static bool isKeyReleased(const char* name) {
        size_t inputCount = inputs.size();
        for (size_t i = 0; i < inputCount; i++) {
            std::pair<std::string_view, bool>* statePair = &states[i];
            std::pair<std::string_view, bool>* oldStatePair = &oldStates[i];
            if (statePair->first == name) return (oldStatePair->second && !statePair->second);
        }

        return false;
    }
};

bool ZInputHandler::constructed = false;
std::vector<std::pair<std::string, std::vector<SDL_Scancode>>> ZInputHandler::inputs = std::vector<std::pair<std::string, std::vector<SDL_Scancode>>>();
std::vector<std::pair<std::string_view, bool>> ZInputHandler::states = std::vector<std::pair<std::string_view, bool>>();
std::vector<std::pair<std::string_view, bool>> ZInputHandler::oldStates = std::vector<std::pair<std::string_view, bool>>();

class ZWindow {
    friend class ZRenderer;

    //The window we'll be rendering to
    SDL_Window* window = NULL;
    int width = 800, height = 500;

    bool shouldClose = false;

public:
    ZWindow() = default;

    ZWindow(float ratio) {
        //Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
            return;
        }

        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
            SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
            return;
        }

        if (ratio == 0) {
            width = mode.w / 3.0f * 2;
            height = mode.h / 3.0f * 2;
        }

        else {
            height = mode.h / 3.0f * 2;
            width = height * ratio;
        }

        //Create window
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
        }
    }

	void start() {
        //Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
        }

        else {
            //Create window
            window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
            if (window == NULL) {
                std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
            }
        }
	}

    IVec2 getSize() {
        return IVec2(width, height);
    }

    bool isShouldClose() {
        return shouldClose;
    }

    void pollEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) shouldClose = true;
        }
        ZInputHandler::update(&e);
    }

    ~ZWindow() {
        //Destroy window
        SDL_DestroyWindow(window);

        //Quit SDL subsystems
        SDL_Quit();
    }
};

class ZRenderer {
    ZRef<ZWindow> window;

    //The window renderer
    SDL_Renderer* renderer = NULL;

    U8Vec4 renderColor = U8Vec4(255);

public:
    ZRenderer() {}
    ZRenderer(ZRef<ZWindow> mainWindow) {
        window = mainWindow;

        if (window->window == NULL) return;

        renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) {
            std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << '\n';
            return;
        }

        setRenderDrawColor(renderColor);

        //Initialize PNG loading
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << SDL_GetError() << '\n';
            return;
        }

        return;
    }

    void present() {
        SDL_RenderPresent(renderer);
    }

    void setViewport(IVec4 coords) {
        SDL_Rect viewport;
        viewport.x = coords.x;
        viewport.y = coords.y;
        viewport.w = coords.z;
        viewport.h = coords.w;

        SDL_RenderSetViewport(renderer, &viewport);
    }

    U8Vec4 getRenderDrawColor() {
        return renderColor;
    }

    void setRenderDrawColor(U8Vec4 color) {
        SDL_SetRenderDrawColor(renderer, color.x, color.y, color.z, color.w);
        renderColor = color;
    }

    void clearScreen() {
        SDL_RenderClear(renderer);
    }

    void fillRect(IVec4 coords) {
        SDL_Rect fillRect = { coords.x, coords.y, coords.z, coords.w };
        SDL_RenderFillRect(renderer, &fillRect);
    }

    void fillRectF(Vec4 coords) {
        SDL_FRect fillRect = { coords.x, coords.y, coords.z, coords.w };
        SDL_RenderFillRectF(renderer, &fillRect);
    }

    void renderTexture(SDL_Texture* texture, IVec4 src, Vec4 dst, float angle) {
        if (texture == NULL) return;

        SDL_Rect srcRect = { src.x, src.y, src.z, src.w };
        SDL_FRect dstRect = { dst.x, dst.y, dst.z, dst.w };

        SDL_RenderCopyExF(renderer, texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE);
    }

    SDL_Texture* loadTexture(SDL_RWops* src, bool freeSrc) {
        return IMG_LoadTexture_RW(renderer, src, freeSrc);
    }

    ~ZRenderer() {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
};

#endif // !ZENGINE_ZRENDERER
