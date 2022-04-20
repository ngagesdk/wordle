/** @file wordlist_utils.c
 *
 *  Wordlist utilities.
 *
 **/

#include <SDL.h>
#include <time.h>
#include "game.h"

void         set_language(const lang_t language, const SDL_bool set_title_screen, game_t* core);
void         set_next_language(game_t* core);
unsigned int get_nyt_daily_index(void);
void         get_valid_answer(char valid_answer[6], game_t* core);
SDL_bool     is_guess_allowed(const char* guess, game_t* core);
void         validate_current_guess(SDL_bool* is_won, game_t* core);

extern const unsigned int  wordlist_en_letter_count;
extern const unsigned int  wordlist_en_word_count;
extern const unsigned int  wordlist_en_allowed_count;
extern const char          wordlist_en_first_letter;
extern const char          wordlist_en_last_letter;
extern const Uint32        wordlist_en_hash[0x90b];
extern const unsigned char wordlist_en[0x90b][5];
extern const Uint32        wordlist_en_allowed_hash[0x29a7];
extern const unsigned char wordlist_en_allowed[0x29a7][5];
extern const SDL_bool      wordlist_en_is_cyrillic;
extern const char          wordlist_en_special_chars[1];
extern const char          wordlist_en_title[5];

extern const unsigned int  wordlist_ru_letter_count;
extern const unsigned int  wordlist_ru_word_count;
extern const char          wordlist_ru_first_letter;
extern const char          wordlist_ru_last_letter;
extern const Uint32        wordlist_ru_hash[0x1039];
extern const unsigned char wordlist_ru[0x1039][5];
extern const SDL_bool      wordlist_ru_is_cyrillic;
extern const char          wordlist_ru_special_chars[1];
extern const char          wordlist_ru_title[5];

extern const unsigned int  wordlist_de_letter_count;
extern const unsigned int  wordlist_de_word_count;
extern const char          wordlist_de_first_letter;
extern const char          wordlist_de_last_letter;
extern const Uint32        wordlist_de_hash[0x185d];
extern const unsigned char wordlist_de[0x185d][5];
extern const SDL_bool      wordlist_de_is_cyrillic;
extern const char          wordlist_de_special_chars[4];
extern const char          wordlist_de_title[5];

extern const unsigned int  wordlist_fi_letter_count;
extern const unsigned int  wordlist_fi_word_count;
extern const char          wordlist_fi_first_letter;
extern const char          wordlist_fi_last_letter;
extern const Uint32        wordlist_fi_hash[0xcc7];
extern const unsigned char wordlist_fi[0xcc7][5];
extern const SDL_bool      wordlist_fi_is_cyrillic;
extern const char          wordlist_fi_special_chars[1];
extern const char          wordlist_fi_title[5];

void set_language(const lang_t language, const SDL_bool set_title_screen, game_t* core)
{
    int index;

    if (NULL == core)
    {
        return;
    }

    if (SDL_TRUE == set_title_screen)
    {
        core->show_menu       = SDL_TRUE;
        core->tile[5].letter  = 'N';
        core->tile[6].letter  = 'G';
        core->tile[7].letter  = 'A';
        core->tile[8].letter  = 'G';
        core->tile[9].letter  = 'E';
        core->tile[10].state  = CORRECT_LETTER;
        core->tile[11].state  = CORRECT_LETTER;
        core->tile[12].state  = CORRECT_LETTER;
        core->tile[13].state  = WRONG_POSITION;
        core->tile[14].state  = WRONG_POSITION;

        core->tile[25].letter = 0x01; // Load game icon
        core->tile[26].letter = 0x02; // New game icon
        core->tile[27].letter = 0x03; // Game mode icon
        core->tile[28].letter = 0x04; // Set lang. icon
        core->tile[29].letter = 0x05; // Quit game icon

        if (SDL_TRUE == core->language_set_once)
        {
            core->tile[23].letter = 0x06; // Flag icon
        }
    }

    core->wordlist.language = language;

    switch (language)
    {
        default:
        case LANG_ENGLISH:
            if (SDL_TRUE == set_title_screen)
            {
                for (index = 10; index <= 14; index += 1)
                {
                    core->tile[index].letter = wordlist_en_title[index - 10];
                }
            }

            core->wordlist.letter_count  = wordlist_en_letter_count;
            core->wordlist.word_count    = wordlist_en_word_count;
            core->wordlist.allowed_count = wordlist_en_allowed_count;
            core->wordlist.first_letter  = wordlist_en_first_letter;
            core->wordlist.last_letter   = wordlist_en_last_letter;
            core->wordlist.is_cyrillic   = wordlist_en_is_cyrillic;
            core->wordlist.special_chars = wordlist_en_special_chars;
            core->wordlist.hash          = wordlist_en_hash;
            core->wordlist.list          = wordlist_en;
            core->wordlist.allowed_hash  = wordlist_en_allowed_hash;
            core->wordlist.allowed_list  = wordlist_en_allowed;
            break;
        case LANG_RUSSIAN:
            if (SDL_TRUE == set_title_screen)
            {
                for (index = 10; index <= 14; index += 1)
                {
                    core->tile[index].letter = wordlist_ru_title[index - 10];
                }
            }

            core->wordlist.letter_count  = wordlist_ru_letter_count;
            core->wordlist.word_count    = wordlist_ru_word_count;
            core->wordlist.allowed_count = 0;
            core->wordlist.first_letter  = wordlist_ru_first_letter;
            core->wordlist.last_letter   = wordlist_ru_last_letter;
            core->wordlist.is_cyrillic   = wordlist_ru_is_cyrillic;
            core->wordlist.special_chars = wordlist_ru_special_chars;
            core->wordlist.hash          = wordlist_ru_hash;
            core->wordlist.list          = wordlist_ru;
            core->wordlist.allowed_hash  = NULL;
            core->wordlist.allowed_list  = NULL;
            break;
        case LANG_GERMAN:
            if (SDL_TRUE == set_title_screen)
            {
                for (index = 10; index <= 14; index += 1)
                {
                    core->tile[index].letter = wordlist_de_title[index - 10];
                }
            }

            core->wordlist.letter_count  = wordlist_de_letter_count;
            core->wordlist.word_count    = wordlist_de_word_count;
            core->wordlist.allowed_count = 0;
            core->wordlist.first_letter  = wordlist_de_first_letter;
            core->wordlist.last_letter   = wordlist_de_last_letter;
            core->wordlist.is_cyrillic   = wordlist_de_is_cyrillic;
            core->wordlist.special_chars = wordlist_de_special_chars;
            core->wordlist.hash          = wordlist_de_hash;
            core->wordlist.list          = wordlist_de;
            core->wordlist.allowed_hash  = NULL;
            core->wordlist.allowed_list  = NULL;
            break;
        case LANG_FINNISH:
            if (SDL_TRUE == set_title_screen)
            {
                for (index = 10; index <= 14; index += 1)
                {
                    core->tile[index].letter = wordlist_fi_title[index - 10];
                }
            }

            core->wordlist.letter_count  = wordlist_fi_letter_count;
            core->wordlist.word_count    = wordlist_fi_word_count;
            core->wordlist.allowed_count = 0;
            core->wordlist.first_letter  = wordlist_fi_first_letter;
            core->wordlist.last_letter   = wordlist_fi_last_letter;
            core->wordlist.is_cyrillic   = wordlist_fi_is_cyrillic;
            core->wordlist.special_chars = wordlist_fi_special_chars;
            core->wordlist.hash          = wordlist_fi_hash;
            core->wordlist.list          = wordlist_fi;
            core->wordlist.allowed_hash  = NULL;
            core->wordlist.allowed_list  = NULL;
            break;
    }
}

void set_next_language(game_t* core)
{
    switch (core->wordlist.language)
    {
        case LANG_ENGLISH:
            set_language(LANG_RUSSIAN, SDL_TRUE, core);
            break;
        case LANG_RUSSIAN:
            set_language(LANG_GERMAN, SDL_TRUE, core);
            break;
        case LANG_GERMAN:
            set_language(LANG_FINNISH, SDL_TRUE, core);
            break;
        case LANG_FINNISH:
            set_language(LANG_ENGLISH, SDL_TRUE, core);
            break;
    }
}

unsigned int get_nyt_daily_index(void)
{
    time_t seconds;
    int    days_since_epoch;
    int    daily_index;

    seconds          = time(NULL);
    days_since_epoch = seconds / (60 * 60 * 24);
    daily_index      = days_since_epoch - 18797u;

    if (daily_index < 0)
    {
        daily_index = 0;
    }
    else if (daily_index > (wordlist_en_word_count - 1))
    {
        unsigned int overlap_index = daily_index % (wordlist_en_word_count - 1);
        daily_index = overlap_index;
    }

    return daily_index;
}

void get_valid_answer(char valid_answer[6], game_t* core)
{
    int index;

    if (NULL == core)
    {
        return;
    }

    for (index = 0; index < 5; index += 1)
    {
        valid_answer[index] = core->wordlist.list[core->valid_answer_index][index];
    }
    valid_answer[5] = 0;
}

SDL_bool is_guess_allowed(const char* guess, game_t* core)
{
    Uint32 guess_hash;

    if (NULL == guess)
    {
        return SDL_FALSE;
    }

    guess_hash = generate_hash(guess);

    if (0x0daa8447 == guess_hash) // NGAGE
    {
        return SDL_TRUE; // ;-)
    }

    {
        int index;
        int start_index = 0;
        int end_index   = core->wordlist.word_count - 1;

        for (index = start_index; index < end_index; index +=1)
        {
            if (guess_hash == core->wordlist.hash[index])
            {
                return SDL_TRUE;
            }
        }

        if ((NULL != core->wordlist.allowed_hash) &&
            (NULL != core->wordlist.allowed_list) &&
            (core->wordlist.allowed_count > 0))
        {
            end_index = core->wordlist.allowed_count;
            for (index = start_index; index < end_index; index +=1)
            {
                if (guess_hash == core->wordlist.allowed_hash[index])
                {
                    return SDL_TRUE;
                }
            }
        }
    }

    return SDL_FALSE;
}

void validate_current_guess(SDL_bool* is_won, game_t* core)
{
    Uint32 guess_hash;

    if (NULL == core)
    {
        return;
    }

    guess_hash = generate_hash(core->current_guess);

    {
        int    letter_index;
        int    answer_index;
        Uint32 answer_hash;
        int    start_index     = 0;
        int    end_index       = core->wordlist.word_count - 1;
        char   valid_answer[6] = { 0 };
        char   shelf[5]        = { 0 };

        for (answer_index = start_index; answer_index < end_index; answer_index +=1)
        {
            if (guess_hash == core->wordlist.hash[answer_index])
            {
                break;
            }
        }

        get_valid_answer(valid_answer, core);
        answer_hash = generate_hash(valid_answer);

        // First check for exact matches.
        for (letter_index = 0; letter_index < 5; letter_index += 1)
        {
            int     tile_index   = (core->current_index - 4) + letter_index;
            tile_t* current_tile = &core->tile[tile_index];

            if (core->current_guess[letter_index] == valid_answer[letter_index])
            {
                current_tile->state        = CORRECT_LETTER;
                shelf[letter_index]        = valid_answer[letter_index];
                valid_answer[letter_index] = 0;
            }
        }

        // Then check for matches in the wrong location.
        for (letter_index = 0; letter_index < 5; letter_index += 1)
        {
            int     tile_index   = (core->current_index - 4) + letter_index;
            tile_t* current_tile = &core->tile[tile_index];

            if (CORRECT_LETTER == current_tile->state)
            {
                continue;
            }
            else
            {
                int temp_index;
                for (temp_index = 0; temp_index < 5; temp_index += 1)
                {
                    if (core->current_guess[letter_index] == valid_answer[temp_index])
                    {
                        current_tile->state = WRONG_POSITION;
                    }
                }
            }
        }

        // Finally, check for wrong letters.
        for (letter_index = 0; letter_index < 5; letter_index += 1)
        {
            int     tile_index   = (core->current_index - 4) + letter_index;
            tile_t* current_tile = &core->tile[tile_index];

            if (LETTER_SELECT == current_tile->state)
            {
                current_tile->state = WRONG_LETTER;
            }
        }

        // Do we have a winner?
        if (guess_hash == answer_hash)
        {
            *is_won = SDL_TRUE;
        }
        else
        {
            *is_won = SDL_FALSE;
        }
    }
}
