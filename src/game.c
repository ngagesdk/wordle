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

static void clear_tiles(SDL_bool clear_state, game_t* core);
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

    set_language(LANG_ENGLISH, (*core));
    srand(time(0));

    (*core)->seed       = (unsigned int)rand();
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xc0; // А
                            end_char   = 0xc3; // Г
                        }
                        else
                        {
                            start_char = 'A';
                            end_char   = 'C';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xc4; // Д
                            end_char   = 0xc7; // З
                        }
                        else
                        {
                            start_char = 'D';
                            end_char   = 'F';
                        }

                        dbgprint("%c", *current_letter);
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xc8; // И
                            end_char   = 0xca; // Л
                        }
                        else
                        {
                            start_char = 'G';
                            end_char   = 'I';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xcb; // М
                            end_char   = 0xcf; // П
                        }
                        else
                        {
                            start_char = 'J';
                            end_char   = 'L';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xd0; // Р
                            end_char   = 0xd3; // У
                        }
                        else
                        {
                            start_char = 'M';
                            end_char   = 'O';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xd4; // Ф
                            end_char   = 0xd7; // Ч
                        }
                        else
                        {
                            start_char = 'P';
                            end_char   = 'S';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xd8; // Ш
                            end_char   = 0xdb; // Ы
                        }
                        else
                        {
                            start_char = 'T';
                            end_char   = 'V';
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
                        if (SDL_TRUE == core->wordlist.is_cyrillic)
                        {
                            start_char = 0xdc; // Ь
                            end_char   = 0xdf; // Я
                        }
                        else
                        {
                            start_char = 'W';
                            end_char   = 'Z';
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
                        start_char = core->wordlist.first_letter;
                        end_char   = core->wordlist.last_letter;

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
                        start_char = core->wordlist.first_letter;
                        end_char   = core->wordlist.last_letter;

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
                                        int phrase_index = core->attempt * 5;

                                        clear_tiles(SDL_FALSE, core);

                                        core->title_screen  = SDL_TRUE;
                                        core->current_index = -1;

                                        core->tile[phrase_index + 0].letter = 'E';
                                        core->tile[phrase_index + 1].letter = 'X';
                                        core->tile[phrase_index + 2].letter = 'A';
                                        core->tile[phrase_index + 3].letter = 'C';
                                        core->tile[phrase_index + 4].letter = 'T';
                                    }
                                    else if (SDL_FALSE == is_won && 5 == core->attempt)
                                    {
                                        char valid_answer[6] = { 0 };

                                        get_valid_answer(valid_answer, core);

                                        clear_tiles(SDL_TRUE, core);
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
                    case SDLK_RIGHT:
                        goto_next_letter(core);
                        break;
                    case SDLK_F1:
                    case SDLK_F2:
                        core->is_running = SDL_FALSE;
                        return 0;
                    case SDLK_SLASH:
                        if (LANG_ENGLISH == core->wordlist.language)
                        {
                            set_language(LANG_RUSSIAN, core);
                        }
                        else
                        {
                            set_language(LANG_ENGLISH, core);
                        }
                        core->title_screen = SDL_TRUE;
                        break;
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

static void clear_tiles(SDL_bool clear_state, game_t* core)
{
    int index;

    if (NULL == core)
    {
        return;
    }

    for (index = 0; index < 30; index += 1)
    {
        core->tile[index].letter = 0;
        if (SDL_TRUE == clear_state)
        {
            core->tile[index].state = LETTER_SELECT;
        }
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

            if (SDL_TRUE == core->wordlist.is_cyrillic)
            {
                src.y += 128;
            }

            if (core->tile[index].letter >= core->wordlist.first_letter && core->tile[index].letter <= core->wordlist.last_letter)
            {
                src.x  = (core->tile[index].letter - core->wordlist.first_letter) * 32;
                src.x += 32;
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
            switch (core->tile[index].state)
            {
                default:
                case LETTER_SELECT:
                    src.y = 0;
                    break;
                case CORRECT_LETTER:
                    src.y = 96;
                    break;
                case WRONG_LETTER:
                    src.y = 32;
                    break;
                case WRONG_POSITION:
                    src.y = 64;
                    break;
            }
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
    int  index;
    char valid_answer[6] = { 0 };

    if (NULL == core)
    {
        return;
    }

    clear_tiles(SDL_TRUE, core);

    core->title_screen  = SDL_FALSE;
    core->attempt       = 0;
    core->current_index = 0;

    core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
    while (core->valid_answer_index < 0 || core->valid_answer_index > core->wordlist.word_count)
    {
        core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
    }

    get_valid_answer(valid_answer, core);
    dbgprint("%s", valid_answer);
}
