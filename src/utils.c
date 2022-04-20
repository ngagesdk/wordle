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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int          load_texture_from_file(const char* file_name, SDL_Texture** texture, game_t* core);
Uint32       generate_hash(const unsigned char* name);
unsigned int xorshift(unsigned int* xs);

extern size_t  size_of_file(const char * path);
extern Uint8  *load_binary_file_from_path(const char * path);

int load_texture_from_file(const char* file_name, SDL_Texture** texture, game_t* core)
{
    Uint8*         resource_buf;
    SDL_RWops*     resource;
    SDL_Surface*   surface;
    unsigned char* image_data;
    int            req_format = STBI_rgb;
    int            width;
    int            height;
    int            orig_format;

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

    image_data = stbi_load_from_memory(resource_buf, size_of_file(file_name), &width, &height, &orig_format, req_format);
    if (NULL == image_data)
    {
        free(resource_buf);
        return 1;
    }
    free(resource_buf);

    surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)image_data, width, height, 24, 3 * width, SDL_PIXELFORMAT_RGB24);

    if (NULL == surface)
    {
        stbi_image_free(image_data);
        return 1;
    }

    *texture = SDL_CreateTextureFromSurface(core->renderer, surface);
    if (NULL == *texture)
    {
        SDL_FreeSurface(surface);
        stbi_image_free(image_data);
        return 1;
    }
    SDL_FreeSurface(surface);
    stbi_image_free(image_data);

    return 0;
}

/* djb2 by Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 */
Uint32 generate_hash(const unsigned char* name)
{
    Uint64 hash = 5381;
    Uint32 c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return (Uint32)(hash & 0xffffffff);
}

unsigned int xorshift(unsigned int* xs)
{
    *xs ^= *xs << 7;
    *xs ^= *xs >> 9;
    *xs ^= *xs << 8;
    return *xs;
}
