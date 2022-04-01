/** @file game.c
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include "game.h"

static int draw_tiles(game_t* core)
{
    int index;

    if (NULL == core)
    {
        return 1;
    }

    if (NULL == core->render_target)
    {
        return 1;
    }

    for (index = 0; index < 25; index += 1)
    {

    }

    return 0;
}

int game_init(const char* resource_file, const char* title, game_t** core)
{
    int status = 0;
    int index;

    *core = (game_t*)calloc(1, sizeof(struct game));
    if (NULL == *core)
    {
        return 1;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        return 1;
    }

    (*core)->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        176, 208,
        SDL_WINDOW_FULLSCREEN);

    if (NULL == (*core)->window)
    {
        return 1;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (NULL == (*core)->renderer)
    {
        SDL_DestroyWindow((*core)->window);
        return 1;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        /* Nothing to do here. */
    }

    init_file_reader(resource_file);

    (*core)->render_target = SDL_CreateTexture(
        (*core)->renderer,
        SDL_PIXELFORMAT_RGB444,
        SDL_TEXTUREACCESS_TARGET,
        176, 208);

    if (NULL == (*core)->render_target)
    {
        return 1;
    }

    if (0 > SDL_SetRenderTarget((*core)->renderer, (*core)->render_target))
    {
        SDL_DestroyTexture((*core)->render_target);
        return 1;
    }
    SDL_SetRenderDrawColor((*core)->renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderClear((*core)->renderer);

    for (index = 0; index < 25; index += 1)
    {
        (*core)->tile[index].letter = '\0';
        (*core)->tile[index].state  = EMPTY;
    }
    (*core)->is_running = SDL_TRUE;

    return status;
}

SDL_bool game_is_running(game_t* core)
{
    if (NULL == core)
    {
        return SDL_FALSE;
    }
    else
    {
        return core->is_running;
    }
}

int game_update(game_t *core)
{
    int          status     = 0;
    Uint32       delta_time = 0;
    const Uint8* keystate   = SDL_GetKeyboardState(NULL);
    SDL_Event    event;

    core->time_b = core->time_a;
    core->time_a = SDL_GetTicks();

    if (core->time_a > core->time_b)
    {
        delta_time = core->time_a - core->time_b;
    }
    else
    {
        delta_time = core->time_b - core->time_a;
    }
    core->time_since_last_frame = delta_time;

    if (keystate[SDL_SCANCODE_RIGHT])
    {
        //
    }

    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                        core->is_running = SDL_FALSE;
                        return 0;
                }
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    default:
                        break;
                }
            }
        }
    }

    status = draw_tiles(core);
    return status;
}

void game_quit(game_t* core)
{
    if (core->render_target)
    {
        SDL_DestroyTexture(core->render_target);
        core->render_target = NULL;
    }

    if (core->window)
    {
        SDL_DestroyWindow(core->window);
    }

    if (core->renderer)
    {
        SDL_DestroyRenderer(core->renderer);
    }

    if (core)
    {
        free(core);
    }

    SDL_Quit();
}
