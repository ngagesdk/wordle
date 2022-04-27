/** @File pfs.c
 *
 *  Packed file system implementation.
 *  Adapted version, originally from The Mistral Report:
 *  https://montyontherun.itch.io/the-mistral-report
 *
 *  Copyright (c) 2019, Daniel Monteiro. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause
 *
 **/

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define kDataPath_MaxLength 256

char mDataPath[kDataPath_MaxLength];

void       init_file_reader(const char * dataFilePath);
size_t     size_of_file(const char * path);
Uint8     *load_binary_file_from_path(const char * path);
SDL_RWops *open_binary_file_from_path(const char * path);

void init_file_reader(const char * dataFilePath)
{
    sprintf(mDataPath, "%s", dataFilePath);
}

size_t size_of_file(const char * path)
{
    SDL_RWops *mDataPack = SDL_RWFromFile(mDataPath, "rb");
    char       buffer[85];
    int        c;
    Uint32     size      = 0;
    Uint32     offset    = 0;
    Uint16     entries   = 0;

    if (1 != SDL_RWread(mDataPack, &entries, 2, 1))
    {
        /* Nothing to do here. */
    }

    for (c = 0; c < entries; ++c)
    {
        Uint8 stringSize = 0;

        if (1 != SDL_RWread(mDataPack, &offset, 4, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &stringSize, 1, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &buffer, stringSize + 1, 1))
        {
            /* Nothing to do here. */
        }

        if (0 == strcmp(buffer, path))
        {
            goto found;
        }
    }

found:
    if (offset == 0)
    {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    SDL_RWseek(mDataPack, offset, RW_SEEK_SET);

    if (1 != SDL_RWread(mDataPack, &size, 4, 1))
    {
        /* Nothing to do here. */
    }

    SDL_RWclose(mDataPack);

    return size;
}

Uint8 *load_binary_file_from_path(const char * path)
{
    SDL_RWops *mDataPack = SDL_RWFromFile(mDataPath, "rb");
    Uint32     offset    = 0;
    Uint16     entries   = 0;
    char       buffer[85];
    int        c;
    Uint32     size      = 0;
    Uint8     *toReturn;

    if (1 != SDL_RWread(mDataPack, &entries, 2, 1))
    {
        /* Nothing to do here. */
    }

    for (c = 0; c < entries; ++c)
    {
        Uint8 stringSize = 0;

        if (1 != SDL_RWread(mDataPack, &offset, 4, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &stringSize, 1, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &buffer, stringSize + 1, 1))
        {
            /* Nothing to do here. */
        }

        if (0 == strcmp(buffer, path))
        {
            goto found;
        }
    }

found:
    if (offset == 0)
    {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    SDL_RWseek(mDataPack, offset, RW_SEEK_SET);

    if (1 != SDL_RWread(mDataPack, &size, 4, 1))
    {
        /* Nothing to do here. */
    }

    toReturn = (Uint8 *) malloc(size);

    if (size != SDL_RWread(mDataPack, toReturn, sizeof(Uint8), size))
    {
        /* Nothing to do here. */
    }

    SDL_RWclose(mDataPack);

    return toReturn;
}

SDL_RWops *open_binary_file_from_path(const char * path)
{
    SDL_RWops *mDataPack = SDL_RWFromFile(mDataPath, "rb");
    Uint32     offset    = 0;
    Uint16     entries   = 0;
    char       buffer[85];
    int        c;
    Uint32     size      = 0;

    if (1 != SDL_RWread(mDataPack, &entries, 2, 1))
    {
        /* Nothing to do here. */
    }

    for (c = 0; c < entries; ++c)
    {
        Uint8 stringSize = 0;

        if (1 != SDL_RWread(mDataPack, &offset, 4, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &stringSize, 1, 1))
        {
            /* Nothing to do here. */
        }

        if (1 != SDL_RWread(mDataPack, &buffer, stringSize + 1, 1))
        {
            /* Nothing to do here. */
        }

        if (0 == strcmp(buffer, path))
        {
            goto found;
        }
    }

    return NULL;

found:
    if (offset == 0)
    {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    SDL_RWseek(mDataPack, offset, RW_SEEK_SET);

    if (1 != SDL_RWread(mDataPack, &size, 4, 1))
    {
        /* Nothing to do here. */
    }

    return mDataPack;
}
