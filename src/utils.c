/** @file utils.c
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include "game.h"

int load_texture_from_file(const char* file_name, SDL_Texture** texture, game_t* core)
{
    Uint8*       resource_buf;
    SDL_RWops*   resource;
    SDL_Surface* surface;

    if (NULL == file_name)
    {
        return 1;
    }

    resource_buf = (Uint8*)load_binary_file_from_path(file_name);
    if (NULL == resource_buf)
    {
        return 1;
    }

    resource = SDL_RWFromConstMem((Uint8*)resource_buf, size_of_file(file_name));
    if (NULL == resource)
    {
        free(resource_buf);
        return 1;
    }

    surface = SDL_LoadBMP_RW(resource, SDL_TRUE);
    if (NULL == surface)
    {
        free(resource_buf);
        return 1;
    }
    free(resource_buf);

    if (0 != SDL_SetSurfaceRLE(surface, 1))
    {
        /* Nothing to do here. */
    }

    *texture = SDL_CreateTextureFromSurface(core->renderer, surface);
    if (NULL == *texture)
    {
        SDL_FreeSurface(surface);
        return 1;
    }
    SDL_FreeSurface(surface);

    return 0;
}
