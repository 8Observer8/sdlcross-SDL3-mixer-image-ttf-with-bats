#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(WITH_IMAGE)
#include <SDL3_image/SDL_image.h>
#endif
#if defined(WITH_MIXER)
#include <SDL3_mixer/SDL_mixer.h>
#endif
#if defined(WITH_TTF)
#include <SDL3_ttf/SDL_ttf.h>
#endif
#if defined(WITH_NET)
#include <SDL3_net/SDL_net.h>
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#define ARRAY_SIZE(ARR) ((sizeof(ARR)) / (sizeof(*(ARR))))

static void show_important_message(int duration, const char *format, ...)
{
#if defined(SDL_PLATFORM_ANDROID)
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    SDL_ShowAndroidToast(buffer, duration, -1, 0, 0);
#else
    va_list ap;
    (void)duration;
    va_start(ap, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format,
        ap);
    va_end(ap);
#endif
}

/* ----------------------------
   Minimal globals for WASM main loop
   ---------------------------- */
static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
#if defined(WITH_IMAGE)
static SDL_Texture *g_imageTex = NULL;
#endif
#if defined(WITH_TTF)
static SDL_Texture *g_textTexture = NULL;
static TTF_Font *g_font = NULL;
#endif
#if defined(WITH_MIXER)
static MIX_Mixer *g_mixer = NULL;
static MIX_Audio *g_audio = NULL;
#endif

static int g_width = 640;
static int g_height = 480;
static int g_fullscreen = 0;
static int g_foreground = 1;
static int g_quit = 0;

/* Keep your COLORS and locations structure identical to original */
static const SDL_Color COLORS[10] = {
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 },
    { 128, 0, 0, 255 },
    { 0, 128, 0, 255 },
    { 0, 0, 128, 255 },
    { 128, 128, 0, 255 },
    { 0, 128, 128, 255 },
    { 128, 0, 128, 255 },
    { 192, 192, 192, 255 },
};

struct Location
{
    int valid;
    SDL_FRect rect;
};
static struct Location g_locations[10];

#ifdef SDL_PLATFORM_ANDROID
#define RECT_W 250
#else
#define RECT_W 50
#endif

/* Forward declaration of the loop function used by Emscripten */
#if defined(__EMSCRIPTEN__)
static void main_loop(void *arg);
#endif

int main(int argc, char *argv[])
{
    int linked_version;

    (void)argc;
    (void)argv;

    linked_version = SDL_GetVersion();
    SDL_Log("We compiled against SDL version %u.%u.%u ...\n", SDL_MAJOR_VERSION,
        SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("But we are linking against SDL version %u.%u.%u.\n",
        SDL_VERSIONNUM_MAJOR(linked_version),
        SDL_VERSIONNUM_MINOR(linked_version),
        SDL_VERSIONNUM_MICRO(linked_version));

    SDL_SetHint("SDL_MIXER_DISABLE_DRFLAC", "1");
    SDL_SetHint("SDL_MIXER_DISABLE_DRMP3", "1");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed (%s)", SDL_GetError());
        return 1;
    }

#if defined(WITH_IMAGE)
    {
        int v = IMG_Version();
        SDL_Log("SDL3_image version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v),
            SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }
#endif

#if defined(WITH_MIXER)
    {
        int v = MIX_Version();
        SDL_Log("SDL3_mixer version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v),
            SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }

    if (!MIX_Init())
    {
        SDL_Log("MIX_Init failed (%s)", SDL_GetError());
        return 1;
    }

    g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (g_mixer == NULL)
    {
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return 1;
    }

    {
        SDL_AudioSpec mixerspec;
        MIX_GetMixerFormat(g_mixer, &mixerspec);
        SDL_Log("Mixer is format %s, %d channels, %d frequency",
            SDL_GetAudioFormatName(mixerspec.format), mixerspec.channels,
            mixerspec.freq);
    }

    SDL_Log("Available MIXER decoders:");
    {
        const int num_decoders = MIX_GetNumAudioDecoders();
        if (num_decoders < 0)
        {
            SDL_Log(" - [error (%s)]", SDL_GetError());
        }
        else if (num_decoders == 0)
        {
            SDL_Log(" - [none]");
        }
        else
        {
            for (int i = 0; i < num_decoders; i++)
            {
                SDL_Log(" - %s", MIX_GetAudioDecoder(i));
            }
        }
    }

    const char *audiofname = NULL;

#ifdef __ANDROID__
    audiofname = "audio/picked-coin-echo-2.wav";
#endif // __ANDROID__

#ifdef __WIN32__
    audiofname = "app/src/main/assets/audio/picked-coin-echo-2.wav";
#endif // __WIN32__

#ifdef __EMSCRIPTEN__
    audiofname = "app/src/main/assets/audio/picked-coin-echo-2.wav";
#endif // __EMSCRIPTEN__

    if (audiofname)
    {
        g_audio = MIX_LoadAudio(g_mixer, audiofname, false);
        if (g_audio == NULL)
        {
            SDL_Log("Failed to load '%s' (%s)", audiofname, SDL_GetError());
        }
        if (g_audio)
        {
            SDL_AudioSpec audiospec;
            MIX_GetAudioFormat(g_audio, &audiospec);
            SDL_Log("%s: %s, %d channel%s, %d freq", audiofname,
                SDL_GetAudioFormatName(audiospec.format),
                audiospec.channels, (audiospec.channels == 1) ? "" : "s",
                audiospec.freq);
        }
    }
#endif

#if defined(WITH_NET)
    {
        int v = NET_Version();
        SDL_Log("SDL3_net version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v),
            SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }
#endif

    int width = 640;
    int height = 480;
    int flags = SDL_WINDOW_RESIZABLE;
#if defined(SDL_PLATFORM_ANDROID)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

    char title[32];

    SDL_snprintf(title, sizeof(title), "An SDL %d.%d.%d window",
        SDL_VERSIONNUM_MAJOR(linked_version),
        SDL_VERSIONNUM_MINOR(linked_version),
        SDL_VERSIONNUM_MICRO(linked_version));
    g_window = SDL_CreateWindow(title, width, height, flags);

    if (g_window == NULL)
    {
        show_important_message(5, "Could not create window %s", SDL_GetError());
        return 1;
    }
    SDL_Log("Window created!");

    g_renderer = SDL_CreateRenderer(g_window, NULL);
    if (g_renderer == NULL)
    {
        show_important_message(5, "Could not create renderer: %s",
            SDL_GetError());
        return 1;
    }
    SDL_Log("Renderer created!");

#if defined(WITH_IMAGE)

    const char *imagefname = NULL;

#ifdef __ANDROID__
    imagefname = "sprites/crate.png";
#endif // __ANDROID__

#ifdef __WIN32__
    imagefname = "app/src/main/assets/sprites/crate.png";
#endif // __WIN32__

#ifdef __EMSCRIPTEN__
    imagefname = "app/src/main/assets/sprites/crate.png";
#endif // __EMSCRIPTEN__

    if (imagefname)
    {
        g_imageTex = IMG_LoadTexture(g_renderer, imagefname);
        if (!g_imageTex)
        {
            SDL_Log("Failed to load %s: %s", imagefname, SDL_GetError());
        }
        else
        {
            SDL_Log("Image loaded successfully!");
        }
    }
#endif

#if defined(WITH_TTF)
    SDL_Log("WITH_TTF");
    if (!TTF_Init())
    {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return 1;
    }

    const char *fontfname = NULL;

#ifdef __ANDROID__
    fontfname = "fonts/arial.ttf";
#endif // __ANDROID__

#ifdef __WIN32__
    fontfname = "app/src/main/assets/fonts/arial.ttf";
#endif // __WIN32__

#ifdef __EMSCRIPTEN__
    fontfname = "app/src/main/assets/fonts/arial.ttf";
#endif // __EMSCRIPTEN__

#ifdef __ANDROID__
    g_font = TTF_OpenFont(fontfname, 120);
#endif // __ANDROID__

#ifdef __WIN32__
    g_font = TTF_OpenFont(fontfname, 70);
#endif // __WIN32__

#ifdef __EMSCRIPTEN__
    g_font = TTF_OpenFont(fontfname, 70);
#endif // __EMSCRIPTEN__

    if (!g_font)
    {
        SDL_Log("Failed to open font: %s", SDL_GetError());
        return 1;
    }

    {
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Surface *textSurface =
            TTF_RenderText_Blended(g_font, "Hello World!", 12, white);
        if (!textSurface)
        {
            SDL_Log("TTF_RenderText_Blended failed: %s", SDL_GetError());
            return 1;
        }

        g_textTexture = SDL_CreateTextureFromSurface(g_renderer, textSurface);
        SDL_DestroySurface(textSurface);
    }
#endif

    /* initialize locations exactly like your original loop did */
    for (size_t i = 0; i < ARRAY_SIZE(g_locations); i++)
    {
        g_locations[i].valid = 0;
        g_locations[i].rect.w = RECT_W;
        g_locations[i].rect.h = RECT_W;
    }

    show_important_message(1, "Entering the loop");

    /* copy width/height into globals used by main_loop if Emscripten */
    g_width = width;
    g_height = height;
    g_fullscreen = 0;
    g_foreground = 1;
    g_quit = 0;

#if defined(__EMSCRIPTEN__)
    /* On WebAssembly we hand control to Emscripten's main loop */
    emscripten_set_main_loop_arg(main_loop, NULL, 0, 1);
    /* When the page is closed / program quits, Emscripten will return control
       as needed. We don't reach the cleanup code here in typical single-page
       apps until we call emscripten_cancel_main_loop(), so we'll provide a
       small cleanup path inside main_loop when g_quit becomes true. */
#else
    /* Original blocking loop for native targets */
    while (!g_quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    g_quit = 1;
                    break;
                case SDL_EVENT_DISPLAY_ORIENTATION:
                    switch (event.display.data1)
                    {
                        case SDL_ORIENTATION_LANDSCAPE:
                            show_important_message(1, "landscape");
                            break;
                        case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                            show_important_message(1, "landscape (flipped)");
                            break;
                        case SDL_ORIENTATION_PORTRAIT:
                            show_important_message(1, "portrait");
                            break;
                        case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                            show_important_message(1, "portrait (flipped)");
                            break;
                    }
                    break;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    width = event.window.data1;
                    height = event.window.data2;
                    g_width = width;
                    g_height = height;
                    break;
                case SDL_EVENT_WINDOW_SHOWN:
                    g_foreground = 1;
                    break;
                case SDL_EVENT_WINDOW_HIDDEN:
                    g_foreground = 0;
                    break;
#if !defined(SDL_PLATFORM_ANDROID)
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "mouse button down: which=%d, [%g, %g]",
                        event.button.which, event.button.x,
                        event.button.y);
                    if (event.button.which < ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.button.which].valid = 1;
                        g_locations[event.button.which].rect.x =
                            event.button.x - RECT_W / 2;
                        g_locations[event.button.which].rect.y =
                            event.button.y - RECT_W / 2;
                    }
#if defined(WITH_MIXER)
                    if (g_audio != NULL && !MIX_PlayAudio(g_mixer, g_audio))
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                            "Failed to play audio (%s)", SDL_GetError());
                    }
#endif
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "mouse button up: which=%d, [%g, %g]",
                        event.button.which, event.button.x,
                        event.button.y);
                    if (event.button.which < ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.button.which].valid = 0;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "mouse move: button=%d", event.motion.which);
                    if (event.button.which < ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.button.which].rect.x =
                            event.motion.x - RECT_W / 2;
                        g_locations[event.button.which].rect.y =
                            event.motion.y - RECT_W / 2;
                    }
                    break;
                case SDL_EVENT_WILL_ENTER_BACKGROUND:
                    g_foreground = 0;
                    break;
                case SDL_EVENT_DID_ENTER_FOREGROUND:
                    g_foreground = 1;
                    break;
#endif
#if defined(SDL_PLATFORM_ANDROID)
                case SDL_EVENT_FINGER_DOWN:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "finger down: fingerID=%d, [%f, %f]",
                        (int)event.tfinger.fingerID, event.tfinger.x,
                        event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.tfinger.fingerID].valid = 1;
                        g_locations[event.tfinger.fingerID].rect.x =
                            g_width * event.tfinger.x - RECT_W / 2;
                        g_locations[event.tfinger.fingerID].rect.y =
                            g_height * event.tfinger.y - RECT_W / 2;
                    }

#if defined(WITH_MIXER)
                    // Play the sound effect
                    MIX_PlayAudio(g_mixer, g_audio);
#endif

                    break;
                case SDL_EVENT_FINGER_UP:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "mouse button up: fingerID=%d, [%f, %f]",
                        (int)event.tfinger.fingerID, event.tfinger.x,
                        event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.tfinger.fingerID].valid = 0;
                    }
                    break;
                case SDL_EVENT_FINGER_MOTION:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "mouse move: button=%d", event.motion.which);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                    {
                        g_locations[event.tfinger.fingerID].rect.x =
                            g_width * event.tfinger.x - RECT_W / 2;
                        g_locations[event.tfinger.fingerID].rect.y =
                            g_height * event.tfinger.y - RECT_W / 2;
                    }
                    break;
                case SDL_EVENT_TERMINATING:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                        "Received SDL_EVENT_TERMINATING");
                    g_quit = 1;
                    break;
#endif
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key)
                    {
                        case SDLK_ESCAPE:
                            g_quit = 1;
                            break;
                        case SDLK_RETURN:
                            if (event.key.mod & SDL_KMOD_ALT)
                            {
                                g_fullscreen = !g_fullscreen;
                                SDL_SetWindowFullscreen(g_window, g_fullscreen);
                            }
                            break;
                    }
                    break;
            }
        }
        if (g_foreground)
        {
            SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
            SDL_RenderClear(g_renderer);

#if defined(WITH_IMAGE)

#ifdef __ANDROID__
            if (g_imageTex)
            {
                SDL_FRect dst = { 50, 50, 512, 512 };
                SDL_RenderTexture(g_renderer, g_imageTex, NULL, &dst);
            }
#endif // __ANDROID__

#ifdef __WIN32__
            if (g_imageTex)
            {
                SDL_FRect dst = { 50, 50, 128, 128 };
                SDL_RenderTexture(g_renderer, g_imageTex, NULL, &dst);
            }
#endif // __WIN32__

#endif

#if defined(WITH_TTF)
            if (g_textTexture)
            {
                float tw, th;
                SDL_GetTextureSize(g_textTexture, &tw, &th);

#ifdef __ANDROID__
                SDL_FRect dst = { 700.0f, 100.0f, tw, th };
#endif // __ANDROID__

#ifdef __WIN32__
                SDL_FRect dst = { 200.0f, 50.0f, tw, th };
#endif // __WIN32__

                SDL_RenderTexture(g_renderer, g_textTexture, NULL, &dst);
            }
#endif

            for (size_t i = 0; i < ARRAY_SIZE(g_locations); i++)
            {
                if (g_locations[i].valid)
                {
                    SDL_SetRenderDrawColor(g_renderer, COLORS[i].r, COLORS[i].g,
                        COLORS[i].b, COLORS[i].a);
                    SDL_RenderFillRect(g_renderer, &g_locations[i].rect);
                }
            }
            SDL_RenderPresent(g_renderer);
        }
        SDL_Delay(10);
    }
#endif /* !__EMSCRIPTEN__ */

    /* Cleanup for native; for Emscripten, cleanup may be invoked from main_loop
     */
#if !defined(__EMSCRIPTEN__)
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);

#if defined(WITH_IMAGE)
    if (g_imageTex)
        SDL_DestroyTexture(g_imageTex);
#endif

#if defined(WITH_TTF)
    if (g_textTexture)
        SDL_DestroyTexture(g_textTexture);
    if (g_font)
        TTF_CloseFont(g_font);
    TTF_Quit();
#endif

#if defined(WITH_MIXER)
    if (g_audio)
        MIX_DestroyAudio(g_audio);
    if (g_mixer)
        MIX_DestroyMixer(g_mixer);
    MIX_Quit();
#endif
    SDL_Quit();
    return 0;
#else
    /* For Emscripten builds we return here and let the browser drive the app.
       Actual cleanup happens when g_quit is set (see main_loop). */
    return 0;
#endif
}

/* The loop body extracted so Emscripten can call it. It mirrors the original
   loop body. It uses the globals defined above. */
#if defined(__EMSCRIPTEN__)
static void main_loop(void *arg)
{
    (void)arg;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                g_quit = 1;
                break;
            case SDL_EVENT_DISPLAY_ORIENTATION:
                switch (event.display.data1)
                {
                    case SDL_ORIENTATION_LANDSCAPE:
                        show_important_message(1, "landscape");
                        break;
                    case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                        show_important_message(1, "landscape (flipped)");
                        break;
                    case SDL_ORIENTATION_PORTRAIT:
                        show_important_message(1, "portrait");
                        break;
                    case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                        show_important_message(1, "portrait (flipped)");
                        break;
                }
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                g_width = event.window.data1;
                g_height = event.window.data2;
                break;
            case SDL_EVENT_WINDOW_SHOWN:
                g_foreground = 1;
                break;
            case SDL_EVENT_WINDOW_HIDDEN:
                g_foreground = 0;
                break;
#if !defined(SDL_PLATFORM_ANDROID)
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "mouse button down: which=%d, [%g, %g]",
                    event.button.which, event.button.x, event.button.y);
                if (event.button.which < ARRAY_SIZE(g_locations))
                {
                    g_locations[event.button.which].valid = 1;
                    g_locations[event.button.which].rect.x =
                        event.button.x - RECT_W / 2;
                    g_locations[event.button.which].rect.y =
                        event.button.y - RECT_W / 2;
                }
#if defined(WITH_MIXER)
                if (g_audio != NULL && !MIX_PlayAudio(g_mixer, g_audio))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "Failed to play audio (%s)", SDL_GetError());
                }
#endif
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "mouse button up: which=%d, [%g, %g]",
                    event.button.which, event.button.x, event.button.y);
                if (event.button.which < ARRAY_SIZE(g_locations))
                {
                    g_locations[event.button.which].valid = 0;
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d",
                    event.motion.which);
                if (event.button.which < ARRAY_SIZE(g_locations))
                {
                    g_locations[event.button.which].rect.x =
                        event.motion.x - RECT_W / 2;
                    g_locations[event.button.which].rect.y =
                        event.motion.y - RECT_W / 2;
                }
                break;
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
                g_foreground = 0;
                break;
            case SDL_EVENT_DID_ENTER_FOREGROUND:
                g_foreground = 1;
                break;
#endif
#if defined(SDL_PLATFORM_ANDROID)
            case SDL_EVENT_FINGER_DOWN:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "finger down: fingerID=%d, [%f, %f]",
                    (int)event.tfinger.fingerID, event.tfinger.x,
                    event.tfinger.y);
                if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                {
                    g_locations[event.tfinger.fingerID].valid = 1;
                    g_locations[event.tfinger.fingerID].rect.x =
                        g_width * event.tfinger.x - RECT_W / 2;
                    g_locations[event.tfinger.fingerID].rect.y =
                        g_height * event.tfinger.y - RECT_W / 2;
                }

#if defined(WITH_MIXER)
                // Play the sound effect
                MIX_PlayAudio(g_mixer, g_audio);
#endif

                break;
            case SDL_EVENT_FINGER_UP:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "mouse button up: fingerID=%d, [%f, %f]",
                    (int)event.tfinger.fingerID, event.tfinger.x,
                    event.tfinger.y);
                if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                {
                    g_locations[event.tfinger.fingerID].valid = 0;
                }
                break;
            case SDL_EVENT_FINGER_MOTION:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d",
                    event.motion.which);
                if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(g_locations))
                {
                    g_locations[event.tfinger.fingerID].rect.x =
                        g_width * event.tfinger.x - RECT_W / 2;
                    g_locations[event.tfinger.fingerID].rect.y =
                        g_height * event.tfinger.y - RECT_W / 2;
                }
                break;
            case SDL_EVENT_TERMINATING:
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "Received SDL_EVENT_TERMINATING");
                g_quit = 1;
                break;
#endif
            case SDL_EVENT_KEY_UP:
                switch (event.key.key)
                {
                    case SDLK_ESCAPE:
                        g_quit = 1;
                        break;
                    case SDLK_RETURN:
                        if (event.key.mod & SDL_KMOD_ALT)
                        {
                            g_fullscreen = !g_fullscreen;
                            SDL_SetWindowFullscreen(g_window, g_fullscreen);
                        }
                        break;
                }
                break;
        }
    }

    if (g_foreground)
    {
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

#if defined(WITH_IMAGE)

#ifdef __ANDROID__
        if (g_imageTex)
        {
            SDL_FRect dst = { 50, 50, 512, 512 };
            SDL_RenderTexture(g_renderer, g_imageTex, NULL, &dst);
        }
#endif // __ANDROID__

#ifdef __WIN32__
        if (g_imageTex)
        {
            SDL_FRect dst = { 50, 50, 128, 128 };
            SDL_RenderTexture(g_renderer, g_imageTex, NULL, &dst);
        }
#endif // __WIN32__

#ifdef __EMSCRIPTEN__
        if (g_imageTex)
        {
            SDL_FRect dst = { 50, 50, 128, 128 };
            SDL_RenderTexture(g_renderer, g_imageTex, NULL, &dst);
        }
#endif // __EMSCRIPTEN__

#endif

#if defined(WITH_TTF)
        if (g_textTexture)
        {
            float tw, th;
            SDL_GetTextureSize(g_textTexture, &tw, &th);
            SDL_FRect dst = { 200.0f, 50.0f, tw, th };
            SDL_RenderTexture(g_renderer, g_textTexture, NULL, &dst);
        }
#endif

        for (size_t i = 0; i < ARRAY_SIZE(g_locations); i++)
        {
            if (g_locations[i].valid)
            {
                SDL_SetRenderDrawColor(g_renderer, COLORS[i].r, COLORS[i].g,
                    COLORS[i].b, COLORS[i].a);
                SDL_RenderFillRect(g_renderer, &g_locations[i].rect);
            }
        }
        SDL_RenderPresent(g_renderer);
    }

    /* Small delay to avoid busy-looping inside the browser */
    SDL_Delay(10);

    /* If quit requested, cancel the Emscripten main loop and cleanup */
    if (g_quit)
    {
        emscripten_cancel_main_loop();
        /* Cleanup (similar to native path) */
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);

#if defined(WITH_IMAGE)
        if (g_imageTex)
            SDL_DestroyTexture(g_imageTex);
#endif

#if defined(WITH_TTF)
        if (g_textTexture)
            SDL_DestroyTexture(g_textTexture);
        if (g_font)
            TTF_CloseFont(g_font);
        TTF_Quit();
#endif

#if defined(WITH_MIXER)
        if (g_audio)
            MIX_DestroyAudio(g_audio);
        if (g_mixer)
            MIX_DestroyMixer(g_mixer);
        MIX_Quit();
#endif
        SDL_Quit();
    }
}
#endif /* __EMSCRIPTEN__ */
