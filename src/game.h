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

#ifndef SAVE_VERSION
#define SAVE_VERSION 3
#endif

#define WINDOW_WIDTH  176u
#define WINDOW_HEIGHT 208u

typedef enum
{
    LETTER_SELECT = 0,
    CORRECT_LETTER,
    WRONG_LETTER,
    WRONG_POSITION,

} state_t;

typedef enum
{
    LANG_ENGLISH = 0,
    LANG_RUSSIAN,
    LANG_GERMAN,
    LANG_FINNISH

} lang_t;

typedef enum
{
    MODE_NYT = 0,
    MODE_ENDLESS

} game_mode_t;

typedef enum
{
    EVENT_NONE = 0,
    EVENT_KEY_0,
    EVENT_KEY_2,
    EVENT_KEY_3,
    EVENT_KEY_4,
    EVENT_KEY_5,
    EVENT_KEY_6,
    EVENT_KEY_7,
    EVENT_KEY_8,
    EVENT_KEY_9,
    EVENT_CONFIRM,
    EVENT_CONFIRM_LETTER,
    EVENT_DELETE_LETTER,
    EVENT_NEXT_LETTER,
    EVENT_PREV_LETTER,
    EVENT_TEXTINPUT,
    EVENT_CONFIRM_ENDLESS_MODE,
    EVENT_CONFIRM_LOAD_GAME,
    EVENT_CONFIRM_NEW_GAME,
    EVENT_CONFIRM_NYT_MODE,
    EVENT_CONFIRM_SET_LANG,
    EVENT_MENU_NEXT,
    EVENT_MENU_PREV,
    EVENT_MENU_SELECT_ENDLESS_MODE,
    EVENT_MENU_SELECT_NYT_MODE,
    EVENT_MENU_SELECT_NEW_GAME,
    EVENT_MENU_SELECT_QUIT,
    EVENT_BACK,
    EVENT_QUIT,
    EVENT_TOGGLE_FS

} event_t;

typedef struct tile
{
    unsigned char letter;
    unsigned int  letter_index;
    state_t       state;

} tile_t;

typedef struct wordlist
{
    lang_t                language;
    unsigned int          letter_count;
    unsigned int          word_count;
    unsigned int          allowed_count;
    unsigned char         first_letter;
    unsigned char         last_letter;
    SDL_bool              is_cyrillic;
    const unsigned char*  special_chars;
    const Uint32*         hash;
    const unsigned char (*list)[5];
    const Uint32*         allowed_hash;
    const unsigned char (*allowed_list)[5];

} wordlist_t;

typedef struct save_state
{
    unsigned int  version;
    tile_t        tile[30];
    int           current_index;
    unsigned char previous_letter;
    unsigned int  valid_answer_index;
    Uint8         attempt;
    unsigned int  seed;
    lang_t        language;

} save_state_t;

typedef struct nyt_save_state
{
    tile_t        tile[30];
    int           current_index;
    unsigned char previous_letter;
    unsigned int  valid_answer_index;
    Uint8         attempt;
    SDL_bool      has_ended;
    Uint8         final_attempt;

} nyt_save_state_t;

typedef struct game
{
    SDL_Renderer*  renderer;
    SDL_Texture*   render_target;
    SDL_Texture*   tile_texture;
    SDL_Texture*   font_texture;
    SDL_Window*    window;
    SDL_Event      event;
    Uint16         render_offset_x;
    Uint16         render_offset_y;
    Uint8          zoom_factor;
    Uint32         time_since_last_frame;
    Uint32         time_a;
    Uint32         time_b;
    SDL_bool       is_running;
    SDL_bool       show_menu;
    SDL_bool       endless_mode;
    SDL_bool       nyt_mode;
    SDL_bool       show_stats;
    SDL_bool       language_set_once;
    SDL_bool       nyt_has_ended;
    SDL_bool       is_fullscreen;
    tile_t         tile[30];
    int            current_index;
    unsigned char  previous_letter;
    unsigned int   valid_answer_index;
    unsigned char  current_guess[6];
    Uint8          attempt;
    Uint8          nyt_final_attempt;
    unsigned int   seed;
    wordlist_t     wordlist;
    game_mode_t    selected_mode;
#ifdef __ANDROID__
    float          swipe_h;
#endif
} game_t;

int      game_init(const char* resource_file, const char* title, game_t** core);
SDL_bool game_is_running(game_t* core);
int      game_update(game_t* core);
void     game_quit(game_t* core);
void     game_save(game_t* core);
void     game_load(SDL_bool load_daily, game_t* core);

#endif /* GAME_H */
