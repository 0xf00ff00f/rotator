#include "demo.h"

#include <SDL/SDL.h>
#include <GL/glew.h>

#include <memory>
#include <cstdio>
#include <cstdlib>

namespace
{
inline void panic(const char *fmt)
{
    std::printf("%s", fmt);
    std::abort();
}

template<typename... Args>
inline void panic(const char *fmt, const Args &...args)
{
    std::printf(fmt, args...);
    std::abort();
}

std::unique_ptr<Demo> demo;
}

static bool processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN: {
            const auto &keysym = event.key.keysym;
            switch (keysym.sym)
            {
            case SDLK_ESCAPE:
                return false;
            default:
                break;
            }
            break;
        }
        case SDL_QUIT:
            return false;
        }
    }
    return true;
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        panic("Video initialization failed: %s", SDL_GetError());
        return 0;
    }

    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info)
    {
        panic("Video query failed: %s\n", SDL_GetError());
        return 0;
    }

    const int width = 800;
    const int height = 400;
    const int bpp = info->vfmt->BitsPerPixel;
    const Uint32 flags = SDL_OPENGL;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (!SDL_SetVideoMode(width, height, bpp, flags))
    {
        panic("Video mode set failled: %s\n", SDL_GetError());
        return 0;
    }

    glewInit();

    demo.reset(new Demo(width, height));
    demo->initialize();

#ifdef __EMSCRIPTEN__
    emscripten_request_animation_frame_loop(
        [](double now, void *) -> EM_BOOL {
            const auto rv = processEvents();
            if (!rv)
                return EM_FALSE;
            const auto elapsed = [now] {
                static double last = 0;
                const auto elapsed = last != 0 ? now - last : 0;
                last = now;
                return elapsed / 1000.0;
            }();
            demo->renderAndStep(elapsed);
            return EM_TRUE;
        },
        nullptr);
#else
    while (processEvents())
    {
        const auto elapsed = [] {
            static Uint32 last = 0;
            const Uint32 now = SDL_GetTicks();
            const Uint32 elapsed = last != 0 ? now - last : 0;
            last = now;
            return static_cast<double>(elapsed) / 1000.0;
        }();
        demo->renderAndStep(elapsed);
        SDL_GL_SwapBuffers();
    }

    demo.reset();

    SDL_Quit();
#endif
}
