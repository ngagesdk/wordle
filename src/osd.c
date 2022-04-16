/** @file osd.c
 *
 *  On-screen display handler.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include "game.h"

static void get_character_position(const unsigned char character, int* pos_x, int* pos_y);

int osd_init(game_t* core)
{
    int status;

    status = load_texture_from_file((const char*)"font.bmp", &core->font_texture, core);
    return status;
}

void osd_print(const char* display_text, const int pos_x, const int pos_y, game_t* core)
{
    int      char_index  = 0;
    size_t   text_length = SDL_strlen(display_text);
    SDL_Rect src         = { 0,         0,         7, 9 };
    SDL_Rect dst         = { pos_x + 1, pos_y + 1, 7, 9 };
    SDL_Rect frame       = { pos_x, pos_y, (text_length * 7) + 2, 11 };
    SDL_Rect background  = { pos_x, pos_y, (text_length * 7) + 2, 11 };

    if (NULL == core)
    {
        return;
    }

    if (NULL == core->font_texture)
    {
        return;
    }

    SDL_SetRenderDrawColor(core->renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderFillRect(core->renderer, &background);
    SDL_RenderDrawRect(core->renderer, &background);
    SDL_SetRenderDrawColor(core->renderer, 0x84, 0x8a, 0x8c, 0x00);
    SDL_RenderDrawRect(core->renderer, &frame);
    SDL_RenderDrawRect(core->renderer, &frame);

    while ('\0' != display_text[char_index])
    {
        get_character_position(display_text[char_index], &src.x, &src.y);
        char_index += 1;

        SDL_RenderCopy(core->renderer, core->font_texture, &src, &dst);
        dst.x += 7;
    }
}

static void get_character_position(const unsigned char character, int* pos_x, int* pos_y)
{
    int index = 0;

    // If the character is not valid, select space.
    if ((character < 0x20) || (character > 0x7e))
    {
        index = 0;
    }
    else
    {
        index = character - 0x20;
    }

    *pos_x = (index % 18) * 7;
    *pos_y = (index / 18) * 9;
}
