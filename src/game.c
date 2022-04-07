/** @file game.c
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include <time.h>
#include "game.h"

static void clear_tiles(game_t* core);
static int  draw_tiles(game_t* core);
static void get_index_limits(int* lower_limit, int* upper_limit, game_t* core);
static void goto_next_letter(game_t* core);
static void delete_letter(game_t* core);
static void reset_game(game_t* core);

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

    (*core)->tile[5].letter  = 'N';
    (*core)->tile[6].letter  = 'G';
    (*core)->tile[7].letter  = 'A';
    (*core)->tile[8].letter  = 'G';
    (*core)->tile[9].letter  = 'E';

    set_language(LANG_ENGLISH, (*core));

    switch ((*core)->wordlist.language)
    {
        case LANG_RUSSIAN:
            (*core)->tile[10].letter = 0xc2; // В
            (*core)->tile[11].letter = 0xce; // О
            (*core)->tile[12].letter = 0xd0; // Р
            (*core)->tile[13].letter = 0xc4; // Д
            (*core)->tile[14].letter = 0xcb; // Л
            break;
        case LANG_ENGLISH:
        default:
            (*core)->tile[10].letter = 'W';
            (*core)->tile[11].letter = 'O';
            (*core)->tile[12].letter = 'R';
            (*core)->tile[13].letter = 'D';
            (*core)->tile[14].letter = 'L';
            break;
    }

    (*core)->tile[10].state  = CORRECT_LETTER;
    (*core)->tile[11].state  = WRONG_POSITION;
    (*core)->tile[12].state  = CORRECT_LETTER;
    (*core)->tile[13].state  = CORRECT_LETTER;
    (*core)->tile[14].state  = CORRECT_LETTER;

    srand(time(0));

    (*core)->seed            = (unsigned int)rand();
    (*core)->title_screen    = SDL_TRUE;
    (*core)->is_running      = SDL_TRUE;

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
        core->tile[0].letter = 0;
    }

    if (SDL_TRUE == core->title_screen)
    {
        redraw_tiles = SDL_TRUE;
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

    if (SDL_PollEvent(&event))
    {
        char*   current_letter   = &core->tile[core->current_index].letter;
        lang_t* current_language = &core->wordlist.language;
        char    start_char;
        char    end_char;

        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                if (SDL_TRUE == core->title_screen)
                {
                    reset_game(core);
                }

                switch (event.key.keysym.sym)
                {
                    case SDLK_2:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xa1; // А
                                end_char   = 0xad; // Г
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'A';
                                end_char   = 'C';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_3:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xa7; // Д
                                end_char   = 0xf8; // З
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'D';
                                end_char   = 'F';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_4:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xdd; // И
                                end_char   = 0xd1; // Л
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'G';
                                end_char   = 'I';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_5:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xd3; // М
                                end_char   = 0xdd; // П
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'J';
                                end_char   = 'L';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_6:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xe2; // Р
                                end_char   = 0xe8; // У
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'M';
                                end_char   = 'O';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_7:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xab; // Ф
                                end_char   = 0xfc; // Ч
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'P';
                                end_char   = 'S';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_8:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xf6; // Ш
                                end_char   = 0xf2; // Ы
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'T';
                                end_char   = 'V';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_9:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xee; // Ь
                                end_char   = 0xe0; // Я
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'W';
                                end_char   = 'Z';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter < end_char)
                        {
                            *current_letter += 1;
                        }
                        else
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_UP:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xa1; // А
                                end_char   = 0xe0; // Я
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'A';
                                end_char   = 'Z';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter <= end_char)
                        {
                            *current_letter += 1;
                            if (*current_letter > end_char)
                            {
                                *current_letter = start_char;
                            }
                        }
                        else if (0 == *current_letter)
                        {
                            *current_letter = start_char;
                        }
                        break;
                    case SDLK_DOWN:
                        switch (*current_language)
                        {
                            case LANG_RUSSIAN:
                                start_char = 0xa1; // А
                                end_char   = 0xe0; // Я
                                break;
                            case LANG_ENGLISH:
                            default:
                                start_char = 'A';
                                end_char   = 'Z';
                                break;
                        }

                        if (*current_letter >= start_char && *current_letter <= end_char)
                        {
                            *current_letter -= 1;
                            if (*current_letter < start_char)
                            {
                                *current_letter = end_char;
                            }
                        }
                        else if (0 == *current_letter)
                        {
                            *current_letter = end_char;
                        }
                        break;
                    case SDLK_RETURN:
                        if (core->attempt < 6)
                        {
                            if (0 == ((core->current_index + 1) % 5))
                            {
                                int  index;
                                int  letter_index;
                                int  end_index;

                                get_index_limits(&index, &end_index, core);

                                for (letter_index = 0; letter_index < 5; letter_index += 1)
                                {
                                    core->current_guess[letter_index]  = core->tile[index].letter;
                                    index                             += 1;
                                }

                                if (SDL_TRUE == is_guess_allowed(core->current_guess, core))
                                {
                                    SDL_bool is_won = SDL_FALSE;
                                    validate_current_guess(&is_won, core);

                                    if (SDL_TRUE == is_won)
                                    {
                                        clear_tiles(core);
                                        core->title_screen  = SDL_TRUE;
                                        core->current_index = -1;

                                        core->tile[5].letter  = 'Y';
                                        core->tile[6].letter  = 'O';
                                        core->tile[7].letter  = 'U';

                                        core->tile[12].letter = 'W';
                                        core->tile[13].letter = 'O';
                                        core->tile[14].letter = 'N';

                                        core->tile[20].letter = core->current_guess[0];
                                        core->tile[21].letter = core->current_guess[1];
                                        core->tile[22].letter = core->current_guess[2];
                                        core->tile[23].letter = core->current_guess[3];
                                        core->tile[24].letter = core->current_guess[4];

                                        core->tile[6].state   = CORRECT_LETTER;
                                        core->tile[13].state  = WRONG_POSITION;

                                        core->tile[20].state  = CORRECT_LETTER;
                                        core->tile[21].state  = CORRECT_LETTER;
                                        core->tile[22].state  = CORRECT_LETTER;
                                        core->tile[23].state  = CORRECT_LETTER;
                                        core->tile[24].state  = CORRECT_LETTER;
                                    }
                                    else if (SDL_FALSE == is_won && 5 == core->attempt)
                                    {
                                        char valid_answer[6] = { 0 };

                                        get_valid_answer(valid_answer, core);

                                        clear_tiles(core);
                                        core->title_screen  = SDL_TRUE;
                                        core->current_index = -1;

                                        core->tile[5].letter  = 'T';
                                        core->tile[6].letter  = 'O';
                                        core->tile[7].letter  = 'O';

                                        core->tile[12].letter = 'B';
                                        core->tile[13].letter = 'A';
                                        core->tile[14].letter = 'D';

                                        core->tile[20].letter = valid_answer[0];
                                        core->tile[21].letter = valid_answer[1];
                                        core->tile[22].letter = valid_answer[2];
                                        core->tile[23].letter = valid_answer[3];
                                        core->tile[24].letter = valid_answer[4];

                                        core->tile[6].state   = CORRECT_LETTER;
                                        core->tile[13].state  = WRONG_POSITION;

                                        core->tile[20].state  = CORRECT_LETTER;
                                        core->tile[21].state  = CORRECT_LETTER;
                                        core->tile[22].state  = CORRECT_LETTER;
                                        core->tile[23].state  = CORRECT_LETTER;
                                        core->tile[24].state  = CORRECT_LETTER;
                                    }
                                    else
                                    {
                                        core->attempt += 1;
                                        goto_next_letter(core);
                                    }
                                }
                                else
                                {
                                    // Word not valid: shake row?
                                }
                            }
                        }
                        break;
                    case SDLK_BACKSPACE:
                    case SDLK_LEFT:
                        delete_letter(core);
                        break;
                    case SDLK_0:
                    case SDLK_ASTERISK:
                    case SDLK_RIGHT:
                        goto_next_letter(core);
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

static void clear_tiles(game_t* core)
{
    int index;

    if (NULL == core)
    {
        return;
    }

    for (index = 0; index < 30; index += 1)
    {
        core->tile[index].letter = 0;
        core->tile[index].state  = LETTER_SELECT;
    }
}

static int draw_tiles(game_t* core)
{
    SDL_Rect src   = { 0, 0, 32, 32 };
    SDL_Rect dst   = { 4, 2, 32, 32 };
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

    for (index = 0; index < 30; index += 1)
    {
        static SDL_bool is_ngage = SDL_FALSE;

        if ((0 == index) || (0 == (index % 5)))
        {
            const int hash_ngage  = 0x0daa8447; // NGAGE
            char check_pattern[6] = { 0 };
            int  letter_index;

            for (letter_index = 0; letter_index < 5; letter_index += 1)
            {
                check_pattern[letter_index] = core->tile[index + letter_index].letter;
            }

            if (hash_ngage == generate_hash(check_pattern))
            {
                is_ngage = SDL_TRUE;
            }
        }

        if (SDL_FALSE == is_ngage)
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

            if (LANG_RUSSIAN == core->wordlist.language)
            {
                switch (core->tile[index].letter)
                {
                    case 0xa1: // А
                        src.x = 0;
                        break;
                    case 0xa2: // Б
                        src.x = 1;
                        break;
                    case 0xec: // В
                        src.x = 2;
                        break;
                    case 0xad: // Г
                        src.x = 3;
                        break;
                    case 0xa7: // Д
                        src.x = 4;
                        break;
                    case 0xa9: // Е
                        src.x = 5;
                        break;
                    case 0xea: // Ж
                        src.x = 6;
                        break;
                    case 0xf4: // З
                        src.x = 7;
                        break;
                    case 0xb8: // И
                        src.x = 8;
                        break;
                    case 0xbe: // Й
                        src.x = 9;
                        break;
                    case 0xc7: // К
                        src.x = 10;
                        break;
                    case 0xd1: // Л
                        src.x = 11;
                        break;
                    case 0xd3: // М
                        src.x = 12;
                        break;
                    case 0xd5: // Н
                        src.x = 13;
                        break;
                    case 0xd7: // О
                        src.x = 14;
                        break;
                    case 0xdd: // П
                        src.x = 15;
                        break;
                    case 0xe2: // Р
                        src.x = 16;
                        break;
                    case 0xe4: // С
                        src.x = 17;
                        break;
                    case 0xe6: // Т
                        src.x = 18;
                        break;
                    case 0xe8: // У
                        src.x = 19;
                        break;
                    case 0xab: // Ф
                        src.x = 20;
                        break;
                    case 0xb6: // Х
                        src.x = 21;
                        break;
                    case 0xa5: // Ц
                        src.x = 22;
                        break;
                    case 0xfc: // Ч
                        src.x = 23;
                        break;
                    case 0xf6: // Ш
                        src.x = 24;
                        break;
                    case 0xfa: // Щ
                        src.x = 25;
                        break;
                    case 0x9f: // Ъ
                        src.x = 26;
                        break;
                    case 0xf2: // Ы
                        src.x = 27;
                        break;
                    case 0xee: // Ь
                        src.x = 28;
                        break;
                    case 0xf8: // Э
                        src.x = 29;
                        break;
                    case 0x9d: // Ю
                        src.x = 30;
                        break;
                    case 0xe0: // Я
                        src.x = 31;
                        break;
                }
                src.x *= 32;
                src.x += 32;
                src.y += 128;
            }
            else
            {
                if (core->tile[index].letter >= 'A' && core->tile[index].letter <= 'Z')
                {
                    src.x  = (core->tile[index].letter - 'A') * 32;
                    src.x += 32;
                }
            }
        }
        else
        {
            src.x = 1024;
            switch(core->tile[index].letter)
            {
                case 'N':
                    src.y = 0;
                    break;
                case 'G':
                    src.y = 32;
                    break;
                case 'A':
                    src.y = 64;
                    break;
                case 'E':
                    src.y    = 96;
                    is_ngage = SDL_FALSE;
                    break;
            }
        }

        if (0 == core->tile[index].letter)
        {
            src.x = 0;
            src.y = 0;
        }

        SDL_RenderCopy(core->renderer, core->tile_texture, &src, &dst);

        if (index == core->current_index && SDL_FALSE == core->title_screen)
        {
            SDL_Rect inner_frame = { dst.x + 1, dst.y + 1, dst.w - 2, dst.h - 2 };
            SDL_SetRenderDrawColor(core->renderer, 0xf5, 0x79, 0x3a, 0x00);
            SDL_RenderDrawRect(core->renderer, &dst);
            SDL_RenderDrawRect(core->renderer, &inner_frame);
        }

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

static void get_index_limits(int* lower_limit, int* upper_limit, game_t* core)
{
    if (NULL == core)
    {
        return;
    }

    switch (core->attempt)
    {
        default:
        case 0:
            *lower_limit = 0;
            *upper_limit = 4;
            break;
        case 1:
            *lower_limit = 5;
            *upper_limit = 9;
            break;
        case 2:
            *lower_limit = 10;
            *upper_limit = 14;
            break;
        case 3:
            *lower_limit = 15;
            *upper_limit = 19;
            break;
        case 4:
            *lower_limit = 20;
            *upper_limit = 24;
            break;
        case 5:
            *lower_limit = 25;
            *upper_limit = 29;
            break;
    }
}

static void goto_next_letter(game_t* core)
{
    int lower_index_limit = 0;
    int upper_index_limit = 0;

    if (NULL == core)
    {
        return;
    }

    get_index_limits(&lower_index_limit, &upper_index_limit, core);

    if (core->tile[core->current_index].letter != 0)
    {
        core->current_index                    += 1;
        core->current_index                     = SDL_clamp(core->current_index, lower_index_limit, upper_index_limit);
        core->tile[core->current_index].letter  = 0;
    }
}

static void delete_letter(game_t* core)
{
    int lower_index_limit = 0;
    int upper_index_limit = 0;

    if (NULL == core)
    {
        return;
    }

    get_index_limits(&lower_index_limit, &upper_index_limit, core);

    core->tile[core->current_index].letter  = 0;
    core->current_index                    -= 1;
    core->current_index                     = SDL_clamp(core->current_index, lower_index_limit, upper_index_limit);
}

static void reset_game(game_t* core)
{
    int index;
    char valid_answer[6] = { 0 };

    if (NULL == core)
    {
        return;
    }

    clear_tiles(core);

    core->title_screen       = SDL_FALSE;
    core->attempt            = 0;
    core->current_index      = 0;

    core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
    while (core->valid_answer_index < 0 || core->valid_answer_index > core->wordlist.word_count)
    {
        core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
    }
    get_valid_answer(valid_answer, core);
    dbgprint("%s", valid_answer);
}
