/** @file pfs.c
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

void init_file_reader(const char * dataFilePath)
{
    sprintf (mDataPath, "%s", dataFilePath);
}

size_t size_of_file(const char * path)
{
    FILE   *mDataPack = fopen(mDataPath, "rb");
    char    buffer[85];
    int     c;
    Uint32  size      = 0;
    Uint32  offset    = 0;
    Uint16  entries   = 0;

    fread(&entries, 2, 1, mDataPack);

    for (c = 0; c < entries; ++c)
    {
        uint8_t stringSize = 0;

        fread(&offset, 4, 1, mDataPack);
        fread(&stringSize, 1, 1, mDataPack);
        fread(&buffer, stringSize + 1, 1, mDataPack);

        if (!strcmp(buffer, path))
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

    fseek(mDataPack, offset, SEEK_SET);
    fread(&size, 4, 1, mDataPack);
    fclose(mDataPack);

    return size;
}

Uint8 *load_binary_file_from_path(const char * path)
{
    FILE   *mDataPack = fopen(mDataPath, "rb");
    Uint32  offset    = 0;
    Uint16  entries   = 0;
    char    buffer[85];
    int     c;
    Uint32  size      = 0;
    Uint8  *toReturn;

    fread(&entries, 2, 1, mDataPack);

    for (c = 0; c < entries; ++c)
    {
        Uint8 stringSize = 0;

        fread(&offset, 4, 1, mDataPack);
        fread(&stringSize, 1, 1, mDataPack);
        fread(&buffer, stringSize + 1, 1, mDataPack);

        if (!strcmp(buffer, path))
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

    fseek(mDataPack, offset, SEEK_SET);

    fread(&size, 4, 1, mDataPack);
    toReturn = (Uint8 *) malloc(size);

    fread(toReturn, sizeof(uint8_t), size, mDataPack);
    fclose(mDataPack);

    return toReturn;
}

FILE *open_binary_file_from_path(const char * path)
{
    FILE   *mDataPack = fopen(mDataPath, "rb");
    Uint32  offset    = 0;
    Uint16  entries   = 0;
    char    buffer[85];
    int     c;
    Uint32  size      = 0;

    fread(&entries, 2, 1, mDataPack);

    for (c = 0; c < entries; ++c)
    {
        Uint8 stringSize = 0;

        fread(&offset, 4, 1, mDataPack);
        fread(&stringSize, 1, 1, mDataPack);
        fread(&buffer, stringSize + 1, 1, mDataPack);

        if (!strcmp(buffer, path))
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

    fseek(mDataPack, offset, SEEK_SET);
    fread(&size, 4, 1, mDataPack);

    return mDataPack;
}
