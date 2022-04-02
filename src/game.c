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

static int draw_tiles(game_t* core);

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

    status = load_texture_from_file((const char*)"tiles.bmp", &(*core)->tile_texture, (*core));
    if (0 != status)
    {
        return status;
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
    int          status       = 0;
    SDL_bool     redraw_tiles = SDL_FALSE;
    SDL_Rect     dst          = { 0, 0, 176, 208 };
    Uint32       delta_time   = 0;
    const Uint8* keystate     = SDL_GetKeyboardState(NULL);
    SDL_Event    event;

    if (0 == core->tile[0].letter)
    {
        redraw_tiles         = SDL_TRUE;
        core->tile[0].letter = 'A';
    }

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
        char* current_letter = &core->tile[core->current_index].letter;

        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_2:
                        if (*current_letter >= 'A' && *current_letter < 'C')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'A';
                        }
                        break;
                    case SDLK_3:
                        if (*current_letter >= 'D' && *current_letter < 'F')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'D';
                        }
                        break;
                    case SDLK_4:
                        if (*current_letter >= 'G' && *current_letter < 'I')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'G';
                        }
                        break;
                    case SDLK_5:
                        if (*current_letter >= 'J' && *current_letter < 'L')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'J';
                        }
                        break;
                    case SDLK_6:
                        if (*current_letter >= 'M' && *current_letter < 'O')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'M';
                        }
                        break;
                    case SDLK_7:
                        if (*current_letter >= 'P' && *current_letter < 'S')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'P';
                        }
                        break;
                    case SDLK_8:
                        if (*current_letter >= 'T' && *current_letter < 'V')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'T';
                        }
                        break;
                    case SDLK_9:
                        if (*current_letter >= 'W' && *current_letter < 'Z')
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = 'W';
                        }
                        break;
                    case SDLK_UP:
                        if (*current_letter >= 'A' && *current_letter <= 'Z')
                        {
                            *current_letter += 1;
                            if (*current_letter > 'Z')
                            {
                                *current_letter = 'A';
                            }
                        }
                        break;
                    case SDLK_DOWN:
                        if (*current_letter >= 'A' && *current_letter <= 'Z')
                        {
                            *current_letter -= 1;
                            if (*current_letter < 'A')
                            {
                                *current_letter = 'Z';
                            }
                        }
                        break;
                    case SDLK_BACKSPACE:
                    case SDLK_LEFT:
                        *current_letter      = 0;
                        core->current_index -= 1;
                        core->current_index  = SDL_clamp(core->current_index, 0, 24);
                        break;
                    case SDLK_0:
                    case SDLK_ASTERISK:
                    case SDLK_RIGHT:
                        if (*current_letter != 0)
                        {
                            core->current_index                    += 1;
                            core->current_index                     = SDL_clamp(core->current_index, 0, 24);
                            core->tile[core->current_index].letter  = 'A';
                        }
                        break;
                    case SDLK_F1:
                    case SDLK_F2:
                        core->is_running = SDL_FALSE;
                        return 0;
                }
                redraw_tiles = SDL_TRUE;
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

    if (SDL_TRUE == redraw_tiles)
    {
        status = draw_tiles(core);
        if (0 != status)
        {
            return status;
        }
    }

    if (0 > SDL_SetRenderTarget(core->renderer, NULL))
    {
        return 1;
    }

    if (0 > SDL_RenderCopy(core->renderer, core->render_target, NULL, &dst))
    {
        return 1;
    }

    SDL_SetRenderDrawColor(core->renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return status;
}

void game_quit(game_t* core)
{
    if (core->tile_texture)
    {
        SDL_DestroyTexture(core->tile_texture);
        core->tile_texture = NULL;
    }

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

static int draw_tiles(game_t* core)
{
    SDL_Rect src   = { 0,  0, 32, 32 };
    SDL_Rect dst   = { 4, 20, 32, 32 };
    int      index = 0;
    int      count = 0;

    if (NULL == core)
    {
        return 0;
    }

    if (NULL == core->render_target)
    {
        return 0;
    }

    if (NULL == core->tile_texture)
    {
        return 0;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->render_target))
    {
        return 1;
    }

    for (index = 0; index < 25; index += 1)
    {
        switch(core->tile[index].state)
        {
            default:
            case LETTER_SELECT:
                src.x = 0;
                src.y = 0;
                break;
            case CORRECT_LETTER:
                src.x = 0;
                src.y = 96;
                break;
            case WRONG_LETTER:
                src.x = 0;
                src.y = 32;
                break;
            case WRONG_POSITION:
                src.x = 0;
                src.y = 64;
                break;
        }

        if (core->tile[index].letter >= 'A' && core->tile[index].letter <= 'Z')
        {
            src.x  = (core->tile[index].letter - 'A') * 32;
            src.x += 32;
        }

        SDL_RenderCopy(core->renderer, core->tile_texture, &src, &dst);
        dst.x += 34;
        count += 1;

        if (count > 4)
        {
            count  = 0;
            dst.x  = 4;
            dst.y += 34;
        }
    }

    return 0;
}
