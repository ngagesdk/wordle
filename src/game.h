/** @file game.h
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef GAME_H
#define GAME_H

#include <SDL.h>

typedef enum
{
    EMPTY = 0,
    CORRECT,
    WRONG_LETTER,
    WRONG_POSITION,
    YET_UNKNOWN

} state_t;

typedef struct tile
{
    char    letter;
    state_t state;

} tile_t;

typedef struct game
{
    SDL_Renderer* renderer;
    SDL_Texture*  render_target;
    SDL_Texture*  tiles;
    SDL_Window*   window;
    Uint32        time_since_last_frame;
    Uint32        time_a;
    Uint32        time_b;
    SDL_bool      is_running;
    tile_t        tile[25];
    int           current_index;

} game_t;

int      game_init(const char* resource_file, const char* title, game_t** core);
SDL_bool game_is_running(game_t* core);
int      game_update(game_t* core);
void     game_quit(game_t* core);

#endif /* GAME_H */
