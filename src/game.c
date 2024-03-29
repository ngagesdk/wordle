/** @file game.c
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#if defined __SYMBIAN32__
#define SAVE_FILE       "C:\\wordle.sav"
#define DAILY_SAVE_FILE "C:\\daily.sav"
#else
#define SAVE_FILE       "wordle.sav"
#define DAILY_SAVE_FILE "daily.sav"
#endif

#if ! SDL_VERSION_ATLEAST(2, 0, 20)
int SDL_isalpha(int x) { return isalpha(x); }
#define SDL_clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#endif

static event_t  get_current_event(game_t* core);
static void     clear_tiles(SDL_bool clear_state, game_t* core);
static void     delete_letter(game_t* core);
static int      draw_tiles(game_t* core);
static void     get_index_limits(int* lower_limit, int* upper_limit, game_t* core);
static void     go_back_to_menu(game_t* core);
static void     goto_next_letter(game_t* core);
static SDL_bool has_game_ended(game_t* core);
static void     move_rows_up(game_t* core);
static void     reset_game(SDL_bool nyt_mode, game_t* core);
static void     select_next_letter(const unsigned char start_char, const unsigned char end_char, game_t* core);
static void     select_previous_letter(const unsigned char start_char, const unsigned char end_char, game_t* core);
static void     select_utf8_letter(const Uint16 *text, game_t* core);
static void     show_results(game_t* core);
static void     set_zoom_factor(game_t* core);
static void     set_render_offset(game_t* core);
static void     toggle_fullscreen(game_t* core);

extern void          init_file_reader(const char * dataFilePath);
extern size_t        size_of_file(const char * path);
extern Uint8        *load_binary_file_from_path(const char * path);
extern int           osd_init(game_t* core);
extern void          osd_print(const char* display_text, const int pos_x, const int pos_y, game_t* core);
extern int           load_texture_from_file(const char* file_name, SDL_Texture** texture, game_t* core);
extern Uint32        generate_hash(const unsigned char* name);
extern unsigned int  xorshift(unsigned int* xs);
extern void          set_language(const lang_t language, const SDL_bool set_title_screen, game_t* core);
extern void          set_next_language(game_t* core);
extern unsigned int  get_nyt_daily_index(void);
extern void          get_valid_answer(unsigned char valid_answer[6], game_t* core);
extern SDL_bool      is_guess_allowed(const unsigned char* guess, game_t* core);
extern void          validate_current_guess(SDL_bool* is_won, game_t* core);

int game_init(const char* resource_file, const char* title, game_t** core)
{
    int    status         = 0;
    Uint32 renderer_flags = SDL_RENDERER_SOFTWARE;

    *core = (game_t*)calloc(1, sizeof(struct game));
    if (NULL == *core)
    {
        return 1;
    }

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        return 1;
    }

#ifndef __SYMBIAN32__
    renderer_flags = SDL_RENDERER_ACCELERATED;
# ifndef __ANDROID__
    renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
#  endif
#endif

    set_zoom_factor((*core));

    (*core)->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH  * (*core)->zoom_factor,
        WINDOW_HEIGHT * (*core)->zoom_factor,
        0);

    if (NULL == (*core)->window)
    {
        return 1;
    }

#ifdef __ANDROID__
    toggle_fullscreen((*core));
#endif

    (*core)->renderer = SDL_CreateRenderer((*core)->window, -1, renderer_flags);
    if (NULL == (*core)->renderer)
    {
        if (NULL == (*core)->renderer)
        {
            SDL_DestroyWindow((*core)->window);
            return 1;
        }
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
        WINDOW_WIDTH  * (*core)->zoom_factor,
        WINDOW_HEIGHT * (*core)->zoom_factor);

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

    status = load_texture_from_file((const char*)"tiles.png", &(*core)->tile_texture, (*core));
    if (0 != status)
    {
        return status;
    }

#ifdef __ANDROID__
    status = load_texture_from_file((const char*)"disclaimer.png", &(*core)->disclaimer_texture, (*core));
    if (0 != status)
    {
        return status;
    }

    {
        SDL_RWops* save_file           = NULL;
        char       save_file_path[256] = { 0 };
        stbsp_snprintf(save_file_path, 256, "%s/%s", SDL_AndroidGetInternalStoragePath(), SAVE_FILE);
        save_file = SDL_RWFromFile(save_file_path, "rb");

        if (NULL == save_file)
        {
            (*core)->show_disclaimer = SDL_TRUE;
        }
        else
        {
            SDL_RWclose(save_file);
        }
    }
#endif

    status = osd_init((*core));
    if (0 != status)
    {
        return status;
    }

    set_language(LANG_ENGLISH, SDL_TRUE, (*core));
    srand(time(0));

    (*core)->seed          = (unsigned int)rand();
    (*core)->current_index = 27;
    (*core)->show_menu     = SDL_TRUE;
    (*core)->is_running    = SDL_TRUE;

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
    int           status       = 0;
    SDL_bool      redraw_tiles = SDL_FALSE;
    SDL_Rect      dst          = { 0, 0, WINDOW_WIDTH * core->zoom_factor, WINDOW_HEIGHT * core->zoom_factor };
    Uint32        delta_time   = 0;
    event_t       event        = get_current_event(core);
    unsigned char start_char;
    unsigned char end_char;

    if (NULL == core)
    {
        return 1;
    }

    if (0 == core->tile[0].letter)
    {
        redraw_tiles         = SDL_TRUE;
        core->tile[0].letter = 0;
    }

    if (SDL_TRUE == core->show_menu)
    {
        core->show_stats = SDL_FALSE;
        redraw_tiles     = SDL_TRUE;
    }

    if (EVENT_NONE != event)
    {
#ifdef __ANDROID__
        core->show_disclaimer = SDL_FALSE;
#endif
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

    switch (event)
    {
        default:
        case EVENT_NONE:
            break;
        case EVENT_KEY_0:
            if (0 != core->wordlist.special_chars[0])
            {
                unsigned char* current_letter = &core->tile[core->current_index].letter;

                start_char = core->wordlist.first_letter;
                end_char   = core->wordlist.last_letter;

                if (*current_letter >= start_char && *current_letter < end_char)
                {
                    *current_letter = core->wordlist.special_chars[0];
                }
                else
                {
                    static unsigned int special_char_index = 0;

                    special_char_index += 1;
                    if (0x00 == core->wordlist.special_chars[special_char_index])
                    {
                        special_char_index = 0;
                    }
                    *current_letter = core->wordlist.special_chars[special_char_index];
                }
            }
            break;
        case EVENT_KEY_2:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_3:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_4:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_5:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_6:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_7:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_8:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_KEY_9:
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
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_CONFIRM:
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
                        if (SDL_TRUE == core->nyt_mode)
                        {
                            game_save(core);
                        }

                        if (SDL_TRUE == has_game_ended(core))
                        {
                            if (SDL_TRUE == core->nyt_mode)
                            {
                                core->nyt_final_attempt = core->attempt + 1;
                                core->nyt_has_ended     = SDL_TRUE;
                                game_save(core);
                            }
                            show_results(core);
                        }
                        else
                        {
                            if ((SDL_TRUE == core->endless_mode) && (4 == core->attempt))
                            {
                                move_rows_up(core);
                                core->current_index -= 5;
                            }
                            else
                            {
                                core->attempt += 1;
                            }
                            goto_next_letter(core);
                        }
                    }
                    else
                    {
                        // Word not valid: shake row?
                    }
                }
            }
            else
            {
                game_save(core);
                clear_tiles(SDL_TRUE, core);
                core->current_index = 27;
                set_language(core->wordlist.language, SDL_TRUE, core);
            }
            break;
        case EVENT_CONFIRM_LETTER:
            goto_next_letter(core);
            break;
        case EVENT_DELETE_LETTER:
            delete_letter(core);
            switch((core->current_index))
            {
                case 0:
                case 5:
                case 10:
                case 15:
                case 20:
                case 25:
                    if (0 == core->tile[core->current_index].letter)
                    {
                        go_back_to_menu(core);
                    }
                    break;
                default:
                    break;
            }
            break;
        case EVENT_NEXT_LETTER:
            start_char = core->wordlist.first_letter;
            end_char   = core->wordlist.last_letter;
            select_next_letter(start_char, end_char, core);
            break;
        case EVENT_PREV_LETTER:
            start_char = core->wordlist.first_letter;
            end_char   = core->wordlist.last_letter;
            select_previous_letter(start_char, end_char, core);
            break;
        case EVENT_TEXTINPUT:
            if (SDL_TRUE != core->show_menu)
            {
                select_utf8_letter((const Uint16*)core->event.text.text, core);
            }
            break;
        case EVENT_CONFIRM_ENDLESS_MODE:
            core->endless_mode = SDL_TRUE;
            core->nyt_mode     = SDL_FALSE;
            reset_game(SDL_FALSE, core);
            break;
        case EVENT_CONFIRM_LOAD_GAME:
            core->endless_mode = SDL_FALSE;
            core->nyt_mode     = SDL_FALSE;
            game_load(SDL_FALSE, core);
            break;
        case EVENT_CONFIRM_NEW_GAME:
            core->endless_mode = SDL_FALSE;
            core->nyt_mode     = SDL_FALSE;
            game_save(core);
            reset_game(SDL_FALSE, core);
            break;
        case EVENT_CONFIRM_NYT_MODE:
            core->endless_mode = SDL_FALSE;
            core->nyt_mode     = SDL_TRUE;
            game_load(SDL_TRUE, core);

            if (SDL_TRUE == core->nyt_has_ended)
            {
                show_results(core);
            }
            break;
        case EVENT_CONFIRM_SET_LANG:
            core->language_set_once = SDL_TRUE;
            set_next_language(core);
            break;
        case EVENT_MENU_NEXT:
            core->current_index += 1;
            if (core->current_index > 29)
            {
                core->current_index = 25;
            }
            break;
        case EVENT_MENU_PREV:
            core->current_index -= 1;
            if (core->current_index < 25)
            {
                core->current_index = 29;
            }
            break;
        case EVENT_MENU_SELECT_ENDLESS_MODE:
            core->selected_mode = MODE_ENDLESS;
            break;
        case EVENT_MENU_SELECT_NYT_MODE:
            core->selected_mode = MODE_NYT;
            break;
        case EVENT_MENU_SELECT_NEW_GAME:
            core->current_index = 25;
            break;
        case EVENT_MENU_SELECT_QUIT:
            core->current_index = 29;
            break;
        case EVENT_BACK:
            go_back_to_menu(core);
            break;
        case EVENT_QUIT:
            core->is_running = SDL_FALSE;
            return 0;
        case EVENT_TOGGLE_FS:
            toggle_fullscreen(core);
            break;
    }

    if (SDL_TRUE == redraw_tiles)
    {
        status = draw_tiles(core);
        if (0 != status)
        {
            return status;
        }

        if (SDL_TRUE == core->show_stats)
        {
            char stats[16]   = { 0 };
            int  stats_pos_x = 64;

            if (get_nyt_daily_index() >= 1000)
            {
                stats_pos_x = 57;
            }

            stbsp_snprintf(stats, 16, "Wordle %u %1u/6", get_nyt_daily_index(), core->nyt_final_attempt);
            osd_print(stats, stats_pos_x, 12, core);
        }
    }

    if (0 > SDL_SetRenderTarget(core->renderer, NULL))
    {
        return 1;
    }

    dst.x += core->render_offset_x;
    dst.y += core->render_offset_y;

    if (0 > SDL_RenderCopy(core->renderer, core->render_target, NULL, &dst))
    {
        return 1;
    }

#ifdef __ANDROID__
    if (SDL_TRUE == core->show_disclaimer)
    {
        if (0 > SDL_RenderCopy(core->renderer, core->disclaimer_texture, NULL, &dst))
        {
            return 1;
        }
    }
#endif

    SDL_SetRenderDrawColor(core->renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return status;
}

void game_quit(game_t* core)
{
#ifdef __ANDROID__
    if (core->disclaimer_texture)
    {
        SDL_DestroyTexture(core->disclaimer_texture);
        core->disclaimer_texture = NULL;
    }
#endif

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

/* Since a compiler often appends padding bytes to structures and
 * because this function does not checks the endianness of the system,
 * this function is by definition not portable.  The worst that can
 * happen, however, is that the resulting save file is not usable across
 * different platforms.
 */
void game_save(game_t* core)
{
    SDL_RWops* save_file;
    int        index;

#ifdef __EMSCRIPTEN__
    return;
#endif

    if (NULL == core)
    {
        return;
    }

    if (SDL_FALSE == core->nyt_mode)
    {
        save_state_t state;

        for (index = 0; index < 30; index += 1)
        {
            state.tile[index].letter       = core->tile[index].letter;
            state.tile[index].letter_index = core->tile[index].letter_index;
            state.tile[index].state        = core->tile[index].state;
        }

        state.version            = SAVE_VERSION;
        state.current_index      = core->current_index;
        state.previous_letter    = core->previous_letter;
        state.valid_answer_index = core->valid_answer_index;
        state.attempt            = core->attempt;
        state.seed               = core->seed;
        state.language           = core->wordlist.language;

#ifdef __ANDROID__
        {
            char save_file_path[256] = { 0 };
            stbsp_snprintf(save_file_path, 256, "%s/%s", SDL_AndroidGetInternalStoragePath(), SAVE_FILE);
            save_file = SDL_RWFromFile(save_file_path, "wb");
        }
#else
        save_file = SDL_RWFromFile(SAVE_FILE, "wb");
#endif
        if (NULL == save_file)
        {
            return;
        }

        SDL_RWwrite(save_file, &state, sizeof(struct save_state), 1);
        SDL_RWclose(save_file);
    }
    else
    {
        nyt_save_state_t state;

        for (index = 0; index < 30; index += 1)
        {
            state.tile[index].letter       = core->tile[index].letter;
            state.tile[index].letter_index = core->tile[index].letter_index;
            state.tile[index].state        = core->tile[index].state;
        }

        state.current_index      = core->current_index;
        state.previous_letter    = core->previous_letter;
        state.valid_answer_index = core->valid_answer_index;
        state.has_ended          = core->nyt_has_ended;
        state.attempt            = core->attempt;
        state.final_attempt      = core->nyt_final_attempt;

#ifdef __ANDROID__
        {
            char save_file_path[256] = { 0 };
            stbsp_snprintf(save_file_path, 256, "%s/%s", SDL_AndroidGetInternalStoragePath(), DAILY_SAVE_FILE);
            save_file = SDL_RWFromFile(save_file_path, "wb");
        }
#else
        save_file = SDL_RWFromFile(DAILY_SAVE_FILE, "wb");
#endif
        if (NULL == save_file)
        {
            return;
        }

        SDL_RWwrite(save_file, &state, sizeof(struct nyt_save_state), 1);
        SDL_RWclose(save_file);
    }
}

void game_load(SDL_bool load_daily, game_t* core)
{
    SDL_RWops* save_file;
    int        index;

    if (NULL == core)
    {
        return;
    }

#ifdef __EMSCRIPTEN__
    if (SDL_TRUE == load_daily)
    {
        reset_game(SDL_TRUE, core);
    }
    else
    {
        reset_game(SDL_FALSE, core);
    }
    return;
#endif

    if (SDL_FALSE == load_daily)
    {
        save_state_t state = { 0 };

#ifdef __ANDROID__
        {
            char save_file_path[256] = { 0 };
            stbsp_snprintf(save_file_path, 256, "%s/%s", SDL_AndroidGetInternalStoragePath(), SAVE_FILE);
            save_file = SDL_RWFromFile(save_file_path, "rb");
        }
#else
        save_file = SDL_RWFromFile(SAVE_FILE, "rb");
#endif
        if (NULL == save_file)
        {
            /* Nothing to do here. */
        }
        else
        {
            if (1 != SDL_RWread(save_file, &state, sizeof(struct save_state), 1))
            {
                /* Nothing to do here. */
            }
            SDL_RWclose(save_file);
        }

        if (SAVE_VERSION != state.version)
        {
            // Do not load outdated or invalid save states.
            return;
        }

        core->show_menu = SDL_FALSE;
        clear_tiles(SDL_TRUE, core);

        for (index = 0; index < 30; index += 1)
        {
            core->tile[index].letter       = state.tile[index].letter;
            core->tile[index].letter_index = state.tile[index].letter_index;
            core->tile[index].state        = state.tile[index].state;
        }

        core->current_index      = state.current_index;
        core->previous_letter    = state.previous_letter;
        core->valid_answer_index = state.valid_answer_index;
        core->attempt            = state.attempt;
        core->seed               = state.seed;

        set_language(state.language, SDL_FALSE, core);
    }
    else
    {
        nyt_save_state_t state = { 0 };

#ifdef __ANDROID__
        {
            char save_file_path[256] = { 0 };
            stbsp_snprintf(save_file_path, 256, "%s/%s", SDL_AndroidGetInternalStoragePath(), DAILY_SAVE_FILE);
            save_file = SDL_RWFromFile(save_file_path, "rb");
        }
#else
        save_file = SDL_RWFromFile(DAILY_SAVE_FILE, "rb");
#endif
        if (NULL == save_file)
        {
            /* Nothing to do here. */
        }
        else
        {
            if (1 != SDL_RWread(save_file, &state, sizeof(struct nyt_save_state), 1))
            {
                /* Nothing to do here. */
            }
            SDL_RWclose(save_file);
        }

        if (state.valid_answer_index != get_nyt_daily_index())
        {
            /* New daily world available. */
            reset_game(SDL_TRUE, core);
        }
        else
        {
            core->show_menu = SDL_FALSE;
            clear_tiles(SDL_TRUE, core);

            for (index = 0; index < 30; index += 1)
            {
                core->tile[index].letter       = state.tile[index].letter;
                core->tile[index].letter_index = state.tile[index].letter_index;
                core->tile[index].state        = state.tile[index].state;
            }

            core->current_index      = state.current_index;
            core->previous_letter    = state.previous_letter;
            core->valid_answer_index = state.valid_answer_index;
            core->attempt            = state.attempt;
            core->nyt_has_ended      = state.has_ended;
            core->nyt_final_attempt  = state.final_attempt;

            set_language(LANG_ENGLISH, SDL_FALSE, core);
        }
    }
    SDL_StartTextInput();
}

static event_t get_current_event(game_t* core)
{
    event_t current_event = EVENT_NONE;

    if (NULL == core)
    {
        return current_event;
    }

    if (SDL_PollEvent(&core->event))
    {
        switch (core->event.type)
        {
            case SDL_QUIT:
                return EVENT_QUIT;
#ifdef __ANDROID__
            case SDL_FINGERMOTION:
                core->swipe_v += core->event.tfinger.dy;
                core->swipe_h += core->event.tfinger.dx;
                break;
            case SDL_FINGERUP:
            case SDL_FINGERDOWN:
                if ((SDL_FINGERDOWN == core->event.type))
                {
                    core->touch_down_timestamp = core->event.tfinger.timestamp;
                }
                if ((SDL_FINGERUP == core->event.type))
                {
                    core->touch_up_timestamp = core->event.tfinger.timestamp;
                }

                if (SDL_TRUE == core->show_menu)
                {
                    Uint32 touch_duration = (core->touch_up_timestamp - core->touch_down_timestamp);

                    if (core->swipe_h >= 0.1f)
                    {
                        core->swipe_h = 0.f;
                        return EVENT_MENU_NEXT;
                    }
                    else if (core->swipe_h <= -0.1f)
                    {
                        core->swipe_h = 0.f;
                        return EVENT_MENU_PREV;
                    }

                    if (core->swipe_v >= 0.1f)
                    {
                        if (27 == core->current_index)
                        {
                            core->swipe_v = 0.f;
                            return EVENT_MENU_SELECT_NYT_MODE;
                        }
                    }
                    else if (core->swipe_v <= -0.1f)
                    {
                        if (27 == core->current_index)
                        {
                            core->swipe_v = 0.f;
                            return EVENT_MENU_SELECT_ENDLESS_MODE;
                        }
                    }

                    if ((touch_duration >= 100) && (touch_duration <= 1000))
                    {
                        switch (core->current_index)
                        {
                            case 25:
                                return EVENT_CONFIRM_NEW_GAME;
                            case 26:
                                return EVENT_CONFIRM_LOAD_GAME;
                            case 27:
                                switch (core->selected_mode)
                                {
                                    default:
                                    case MODE_NYT:
                                        return EVENT_CONFIRM_NYT_MODE;
                                    case MODE_ENDLESS:
                                        return EVENT_CONFIRM_ENDLESS_MODE;
                                }
                            case 28:
                                return EVENT_CONFIRM_SET_LANG;
                            case 29:
                                return EVENT_QUIT;
                            default:
                                break;
                        }
                    }
                    break;
                }
                else
                {
                    Uint32 touch_duration = (core->touch_up_timestamp - core->touch_down_timestamp);
                    if ((touch_duration >= 100) && (touch_duration <= 1000))
                    {
                        SDL_StartTextInput();
                    }
                }
                break;
#endif
            case SDL_KEYDOWN:
            {
#ifndef __SYMBIAN32__
                switch (core->event.key.keysym.sym)
                {
                    case SDLK_F11:
                        return EVENT_TOGGLE_FS;
                }
#endif
                if (SDL_TRUE == core->show_menu)
                {
                    switch (core->event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                        case SDLK_SELECT:
                        case SDLK_5:
                        case SDLK_KP_5:
                            switch (core->current_index)
                            {
                                case 25:
                                    return EVENT_CONFIRM_NEW_GAME;
                                case 26:
                                    return EVENT_CONFIRM_LOAD_GAME;
                                case 27:
                                    switch (core->selected_mode)
                                    {
                                        default:
                                        case MODE_NYT:
                                            return EVENT_CONFIRM_NYT_MODE;
                                        case MODE_ENDLESS:
                                            return EVENT_CONFIRM_ENDLESS_MODE;
                                    }
                                case 28:
                                    return EVENT_CONFIRM_SET_LANG;
                                case 29:
                                    return EVENT_QUIT;
                                default:
                                    break;
                            }
                            break;
                        case SDLK_LEFT:
                            return EVENT_MENU_PREV;
                        case SDLK_RIGHT:
                            return EVENT_MENU_NEXT;
                        case SDLK_UP:
                            if (27 == core->current_index)
                            {
                                return EVENT_MENU_SELECT_ENDLESS_MODE;
                            }
                            break;
                        case SDLK_DOWN:
                            if (27 == core->current_index)
                            {
                                return EVENT_MENU_SELECT_NYT_MODE;
                            }
                            break;
#ifdef __SYMBIAN32__
                        case SDLK_SOFTLEFT:
                            return EVENT_MENU_SELECT_NEW_GAME;
                        case SDLK_SOFTRIGHT:
                            return EVENT_MENU_SELECT_QUIT;
#endif
                        case SDLK_AC_BACK:
                        case SDLK_ESCAPE:
                            SDL_StopTextInput();
                            return EVENT_QUIT;
                    }
                }
                else
                {
                    // Interpret key-presses while being not in the
                    // menu.
                    switch (core->event.key.keysym.sym)
                    {
                        case SDLK_2:
                        case SDLK_KP_2:
                            return EVENT_KEY_2;
                        case SDLK_3:
                        case SDLK_KP_3:
                            return EVENT_KEY_3;
                        case SDLK_4:
                        case SDLK_KP_4:
                            return EVENT_KEY_4;
                        case SDLK_5:
                        case SDLK_KP_5:
                            return EVENT_KEY_5;
                        case SDLK_6:
                        case SDLK_KP_6:
                            return EVENT_KEY_6;
                        case SDLK_7:
                        case SDLK_KP_7:
                            return EVENT_KEY_7;
                        case SDLK_8:
                        case SDLK_KP_8:
                            return EVENT_KEY_8;
                        case SDLK_9:
                        case SDLK_KP_9:
                            return EVENT_KEY_9;
                        case SDLK_0:
                        case SDLK_KP_0:
                            return EVENT_KEY_0;
                        case SDLK_UP:
                            return EVENT_NEXT_LETTER;
                        case SDLK_DOWN:
                            return EVENT_PREV_LETTER;
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                        case SDLK_SELECT:
                            return EVENT_CONFIRM;
                        case SDLK_BACKSPACE:
                        case SDLK_LEFT:
                            return EVENT_DELETE_LETTER;
                        case SDLK_RIGHT:
                            return EVENT_CONFIRM_LETTER;
                        case SDLK_AC_BACK:
                        case SDLK_ESCAPE:
#ifdef __SYMBIAN32__
                        case SDLK_SOFTLEFT:
                        case SDLK_SOFTRIGHT:
#endif
                            SDL_StopTextInput();
                            return EVENT_BACK;
                    }
                }
                break;
            }
            case SDL_TEXTINPUT:
                return EVENT_TEXTINPUT;
                break;
        }
    }

    return EVENT_NONE;
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

static void delete_letter(game_t* core)
{
    int lower_index_limit = 0;
    int upper_index_limit = 0;

    if (NULL == core)
    {
        return;
    }

    if (core->attempt >= 6)
    {
        return;
    }

    get_index_limits(&lower_index_limit, &upper_index_limit, core);

    core->tile[core->current_index].letter  = 0;
    core->current_index                    -= 1;
    core->current_index                     = SDL_clamp(core->current_index, lower_index_limit, upper_index_limit);
}

static int draw_tiles(game_t* core)
{
    SDL_Rect src   = { 0, 0, 32, 32 };
    SDL_Rect dst   = { 4 * core->zoom_factor, 2 * core->zoom_factor, 32 * core->zoom_factor, 32 * core->zoom_factor};
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
    SDL_RenderClear(core->renderer);

    for (index = 0; index < 30; index += 1)
    {
        static SDL_bool is_ngage = SDL_FALSE;

        if ((0 == index) || (0 == (index % 5)))
        {
            const Uint32  hash_ngage       = 0x0daa8447; // NGAGE
            unsigned char check_pattern[6] = { 0 };
            int           letter_index;

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

            // Special characters.
            if (SDL_FALSE == core->wordlist.is_cyrillic)
            {
                switch (core->tile[index].letter)
                {
                    case 0xc4: // Ä
                        src.x = 864;
                        break;
                    case 0xd6: // Ö
                        src.x = 896;
                        break;
                    case 0xdc: // Ü
                        src.x = 928;
                        break;
                    case 0xdf: // ß
                        src.x = 960;
                        break;
                }
            }

            switch (core->tile[index].letter)
            {
                case 0x00: // Empty tile
                    src.x = 0;
                    break;
                case 0x2d: // Hyphen
                    src.x = 992;
                    break;
                case 0x01: // New game icon
                    src.x = 0;
                    src.y = 160;
                    break;
                case 0x02: // Load game icon
                    src.x = 0;
                    src.y = 128;
                    break;
                case 0x03: // Game mode icon
                    src.x = 1056;

                    switch(core->selected_mode)
                    {
                        default:
                        case MODE_NYT:
                            src.y = 64;
                            break;
                        case MODE_ENDLESS:
                            src.y = 0;
                            break;
                    }

                    if (27 == core->current_index)
                    {
                        src.y += 32;
                    }
                    break;
                case 0x04: // Set lang. icon
                    src.x = 0;
                    src.y = 192;
                    break;
                case 0x05: // Quit game icon
                    src.x = 0;
                    src.y = 224;
                    break;
                case 0x06: // Flag icon
                    src.x = 1056;
                    switch (core->wordlist.language)
                    {
                        default:
                        case LANG_ENGLISH:
                            src.y = 128;
                            break;
                        case LANG_RUSSIAN:
                            src.y = 160;
                            break;
                        case LANG_GERMAN:
                            src.y = 192;
                            break;
                        case LANG_FINNISH:
                            src.y = 224;
                            break;
                    }
                    break;
            }

            if (SDL_TRUE == core->wordlist.is_cyrillic &&
                0x00     != core->tile[index].letter   &&
                0x2d     != core->tile[index].letter   &&
                0x01     != core->tile[index].letter   &&
                0x02     != core->tile[index].letter   &&
                0x03     != core->tile[index].letter   &&
                0x04     != core->tile[index].letter   &&
                0x05     != core->tile[index].letter   &&
                0x06     != core->tile[index].letter)
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

        SDL_RenderCopy(core->renderer, core->tile_texture, &src, &dst);

        if (index == core->current_index)
        {
            unsigned int frame_count = 0;
            SDL_SetRenderDrawColor(core->renderer, 0xf5, 0x79, 0x3a, 0x00);
            for (frame_count = 0; frame_count < (core->zoom_factor * 2); frame_count += 1)
            {
                SDL_Rect inner_frame = {
                    (dst.x + frame_count),
                    (dst.y + frame_count),
                    (dst.w - (frame_count * 2)),
                    (dst.h - (frame_count * 2))
                };
                SDL_RenderDrawRect(core->renderer, &inner_frame);
            }
        }

        dst.x += (34 * core->zoom_factor);
        count += 1;

        if (count > 4)
        {
            count  = 0;
            dst.x  = (4  * core->zoom_factor);
            dst.y += (34 * core->zoom_factor);
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

static void go_back_to_menu(game_t* core)
{
    SDL_StopTextInput();
    game_save(core);
    clear_tiles(SDL_TRUE, core);
    core->current_index = 27;
    set_language(core->wordlist.language, SDL_TRUE, core);
}

static void goto_next_letter(game_t* core)
{
    int lower_index_limit = 0;
    int upper_index_limit = 0;

    if (NULL == core)
    {
        return;
    }

    if (core->attempt >= 6)
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

static SDL_bool has_game_ended(game_t* core)
{
    SDL_bool is_won = SDL_FALSE;
    validate_current_guess(&is_won, core);

    if ((SDL_TRUE == core->nyt_mode) && (core->nyt_has_ended))
    {
        return SDL_TRUE;
    }

    if ((SDL_TRUE == is_won) || (SDL_FALSE == is_won && 5 == core->attempt))
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

static void move_rows_up(game_t* core)
{
    int counter;
    int tile_index;
    for (counter = 0; counter < 5; counter += 1)
    {
        for (tile_index = 1; tile_index < 30; tile_index += 1)
        {
            core->tile[tile_index - 1].letter       = core->tile[tile_index].letter;
            core->tile[tile_index - 1].letter_index = core->tile[tile_index].letter_index;
            core->tile[tile_index - 1].state        = core->tile[tile_index].state;
        }
    }
}

static void reset_game(SDL_bool nyt_mode, game_t* core)
{
    if (NULL == core)
    {
        return;
    }

    clear_tiles(SDL_TRUE, core);

    core->attempt       = 0;
    core->current_index = 0;
    core->show_menu     = SDL_FALSE;

    if (SDL_TRUE == core->show_stats)
    {
        core->show_stats = SDL_FALSE;
        draw_tiles(core);
    }

    if (SDL_FALSE == nyt_mode)
    {
        core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
        while (core->valid_answer_index > core->wordlist.word_count)
        {
            core->valid_answer_index = xorshift(&core->seed) % core->wordlist.word_count;
        }
        core->nyt_mode = SDL_FALSE;
    }
    else
    {
        set_language(LANG_ENGLISH, SDL_FALSE, core);
        core->valid_answer_index = get_nyt_daily_index();
        core->nyt_mode           = SDL_TRUE;
    }
    SDL_StartTextInput();
}

static void select_next_letter(const unsigned char start_char, const unsigned char end_char, game_t* core)
{
    unsigned char* current_letter = &core->tile[core->current_index].letter;

    if (core->attempt >= 6)
    {
        return;
    }

    if (*current_letter >= start_char && *current_letter < end_char)
    {
        *current_letter += 1;
    }
    else
    {
        *current_letter = start_char;
    }
}

static void select_previous_letter(const unsigned char start_char, const unsigned char end_char, game_t* core)
{
    unsigned char* current_letter = &core->tile[core->current_index].letter;

    if (core->attempt >= 6)
    {
        return;
    }

    if (0 == *current_letter)
    {
        *current_letter = start_char;
    }

    *current_letter -= 1;

    if (*current_letter < start_char)
    {
        *current_letter = end_char;
    }
}

static void select_utf8_letter(const Uint16 *text, game_t* core)
{
    unsigned char* current_letter  = (unsigned char*)&core->tile[core->current_index].letter;
    unsigned char  selected_letter = 0;
    SDL_bool       is_alpha        = SDL_FALSE;

    switch(*text)
    {
        case 0xa4c3: // ä
        case 0x84c3: // Ä
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc4;
            break;
        case 0xb6c3: // ö
        case 0x96c3: // Ö
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd6;
            break;
        case 0xbcc3: // ü
        case 0x9cc3: // Ü
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdc;
            break;
        case 0x9fc3: // ß
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdf;
            break;
        case 0xb0d0: // а
        case 0x90d0: // А
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc0;
            break;
        case 0xb1d0: // б
        case 0x91d0: // Б
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc1;
            break;
        case 0xb2d0: // в
        case 0x92d0: // В
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc2;
            break;
        case 0xb3d0: // г
        case 0x93d0: // Г
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc3;
            break;
        case 0xb4d0: // д
        case 0x94d0: // Д
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc4;
            break;
        case 0xb5d0: // е
        case 0x95d0: // Е
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc5;
            break;
        case 0xb6d0: // ж
        case 0x96d0: // Ж
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc6;
            break;
        case 0xb7d0: // з
        case 0x97d0: // З
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc7;
            break;
        case 0xb8d0: // и
        case 0x98d0: // И
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc8;
            break;
        case 0xb9d0: // й
        case 0x99d0: // Й
            is_alpha        = SDL_TRUE;
            selected_letter = 0xc9;
            break;
        case 0xbad0: // к
        case 0x9ad0: // К
            is_alpha        = SDL_TRUE;
            selected_letter = 0xca;
            break;
        case 0xbbd0: // л
        case 0x9bd0: // Л
            is_alpha        = SDL_TRUE;
            selected_letter = 0xcb;
            break;
        case 0xbcd0: // м
        case 0x9cd0: // М
            is_alpha        = SDL_TRUE;
            selected_letter = 0xcc;
            break;
        case 0xbdd0: // н
        case 0x9dd0: // Н
            is_alpha        = SDL_TRUE;
            selected_letter = 0xcd;
            break;
        case 0xbed0: // о
        case 0x9ed0: // О
            is_alpha        = SDL_TRUE;
            selected_letter = 0xce;
            break;
        case 0xbfd0: // п
        case 0x9fd0: // П
            is_alpha        = SDL_TRUE;
            selected_letter = 0xcf;
            break;
        case 0x80d1: // р
        case 0xa0d0: // Р
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd0;
            break;
        case 0x81d1: // с
        case 0xa1d0: // С
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd1;
            break;
        case 0x82d1: // т
        case 0xa2d0: // Т
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd2;
            break;
        case 0x83d1: // у
        case 0xa3d0: // У
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd3;
            break;
        case 0x84d1: // ф
        case 0xa4d0: // Ф
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd4;
            break;
        case 0x85d1: // х
        case 0xa5d0: // Х
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd5;
            break;
        case 0x86d1: // ц
        case 0xa6d0: // Ц
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd6;
            break;
        case 0x87d1: // ч
        case 0xa7d0: // Ч
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd7;
            break;
        case 0x88d1: // ш
        case 0xa8d0: // Ш
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd8;
            break;
        case 0x89d1: // щ
        case 0xa9d0: // Щ
            is_alpha        = SDL_TRUE;
            selected_letter = 0xd9;
            break;
        case 0x8ad1: // ъ
        case 0xaad0: // Ъ
            is_alpha        = SDL_TRUE;
            selected_letter = 0xda;
            break;
        case 0x8bd1: // ы
        case 0xabd0: // Ы
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdb;
            break;
        case 0x8cd1: // ь
        case 0xacd0: // Ь
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdc;
            break;
        case 0x8dd1: // э
        case 0xadd0: // Э
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdd;
            break;
        case 0x8ed1: // ю
        case 0xaed0: // Ю
            is_alpha        = SDL_TRUE;
            selected_letter = 0xde;
            break;
        case 0x8fd1: // я
        case 0xafd0: // Я
            is_alpha        = SDL_TRUE;
            selected_letter = 0xdf;
            break;
        case 0x2d: // -
            is_alpha        = SDL_TRUE;
            selected_letter = 0x2d;
            break;
        default:
            if (SDL_isalpha(*text))
            {
                is_alpha        = SDL_TRUE;
                selected_letter = (unsigned char)SDL_toupper(*text);
            }
            break;
    }

    if (SDL_TRUE == core->wordlist.is_cyrillic)
    {
        if (((selected_letter < 0xc0) || (selected_letter > 0xdf)) && (selected_letter != 0x2d))
        {
            // Only accept cyrillic letters and hypen in cyrillic
            // wordlists.
            return;
        }
    }
    else
    {
        if (((selected_letter >= 0xc0) && (selected_letter <= 0xdf)))
        {
            // Do not accept cyrillic letters in non-cyrillic wordlists.
            return;
        }
    }

    *current_letter = selected_letter;

    if (0 != ((core->current_index + 1) % 5))
    {
        if (SDL_TRUE == is_alpha)
        {
            core->current_index++;
        }
    }
}

static void show_results(game_t* core)
{
    unsigned char valid_answer[6] = { 0 };

    get_valid_answer(valid_answer, core);
    clear_tiles(SDL_FALSE, core);

    if (SDL_TRUE == core->nyt_mode)
    {
        core->show_stats = SDL_TRUE;
    }
    else
    {
        unsigned int start_index = core->attempt * 5;

        core->tile[start_index].letter     = valid_answer[0];
        core->tile[start_index + 1].letter = valid_answer[1];
        core->tile[start_index + 2].letter = valid_answer[2];
        core->tile[start_index + 3].letter = valid_answer[3];
        core->tile[start_index + 4].letter = valid_answer[4];
    }

    core->attempt       = 6;
    core->current_index = -1;
}

static void set_zoom_factor(game_t* core)
{
    SDL_DisplayMode display_mode;

    core->zoom_factor = 1u;

#if __SYMBIAN32__
    return;
#endif

    if (0 == SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        Uint8 zf_width  = (display_mode.w / WINDOW_WIDTH)  - 1;
        Uint8 zf_height = (display_mode.h / WINDOW_HEIGHT) - 1;

        if (zf_width <= zf_height)
        {
            core->zoom_factor = zf_width;
        }
        else
        {
            core->zoom_factor = zf_height;
        }
    }
}

static void set_render_offset(game_t* core)
{
    SDL_DisplayMode display_mode;

    if (0 == SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        core->render_offset_x = (display_mode.w / 2) - ((WINDOW_WIDTH  * core->zoom_factor) / 2);
        core->render_offset_y = (display_mode.h / 2) - ((WINDOW_HEIGHT * core->zoom_factor) / 2);
        #if __ANDROID__
        core->render_offset_y /= 2;
        #endif
    }
}

static void toggle_fullscreen(game_t* core)
{
    if(core->is_fullscreen)
    {
        if (0 != SDL_SetWindowFullscreen(core->window, 0))
        {
            /* Nothing to do here. */
        }
        else
        {
            core->render_offset_x = 0;
            core->render_offset_y = 0;
            core->is_fullscreen   = !core->is_fullscreen;
        }
    }
    else
    {
        if (0 != SDL_SetWindowFullscreen(core->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
        {
            /* Nothing to do here. */
        }
        else
        {
            set_render_offset(core);
            core->is_fullscreen = !core->is_fullscreen;
            set_zoom_factor(core);
        }
    }
}
