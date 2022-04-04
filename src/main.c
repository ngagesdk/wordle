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

#if defined __SYMBIAN32__
#define RES_FILE "E:\\System\\Apps\\wordle\\data.pfs"
#else
#define RES_FILE "data.pfs"
#endif

int main(int argc, char *argv[])
{
    int     status = 0;
    game_t* core   = NULL;

    status = game_init(RES_FILE, "Wordle", &core);
    if (0 != status)
    {
        goto quit;
    }

    while (game_is_running(core))
    {
        status = game_update(core);
        if (0 != status)
        {
            goto quit;
        }
    }

quit:
    game_quit(core);
    return status;
}
