/** @file main.c
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include "game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

void main_loop_iter(void *core)
{
    int status = game_update((game_t*)core);
    if (0 != status)
    {
        emscripten_cancel_main_loop();
    }
}
#endif

#if defined __SYMBIAN32__
#define RES_FILE "E:\\System\\Apps\\wordle\\data.pfs"
#else
#define RES_FILE "data.pfs"
#endif

int main(int argc, char *argv[])
{
    int     status = 0;
    game_t* core   = NULL;

    (void)argc;
    (void)argv;

    status = game_init(RES_FILE, "Wordle", &core);
    if (0 != status)
    {
        goto quit;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop_iter, core, -1, 1);
#else
    while (game_is_running(core))
    {
        status = game_update(core);
        if (0 != status)
        {
            goto quit;
        }
    }
#endif

quit:
    game_quit(core);
    return status;
}
