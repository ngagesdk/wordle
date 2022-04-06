/** @file wordlist_utils.c
 *
 *  Wordlist utilities.
 *
 **/

#include <SDL.h>
#include "game.h"

void     set_language(const lang_t language, game_t* core);
void     get_valid_answer(char valid_answer[6], game_t* core);
SDL_bool is_guess_allowed(const char* guess, game_t* core);
void     validate_current_guess(SDL_bool* is_won, game_t* core);

extern const unsigned int  wordlist_en_letter_count;
extern const unsigned int  wordlist_en_word_count;
extern const Uint32        wordlist_en_hash[0x90b];
extern const unsigned char wordlist_en[0x90b][6];
extern const unsigned int  wordlist_en_lookup[26];
extern const unsigned int  wordlist_en_offset[27];

extern const unsigned int  wordlist_ru_letter_count;
extern const unsigned int  wordlist_ru_word_count;
extern const Uint32        wordlist_ru_hash[0x1039];
extern const unsigned char wordlist_ru[0x1039][6];
extern const unsigned int  wordlist_ru_lookup[32];
extern const unsigned int  wordlist_ru_offset[33];

void set_language(const lang_t language, game_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->wordlist.language = language;

    switch (language)
    {
        default:
        case LANG_ENGLISH:
            core->wordlist.letter_count = wordlist_en_letter_count;
            core->wordlist.word_count   = wordlist_en_word_count;
            core->wordlist.hash         = wordlist_en_hash;
            core->wordlist.list         = wordlist_en;
            core->wordlist.lookup       = wordlist_en_lookup;
            core->wordlist.offset       = wordlist_en_offset;
            break;
        case LANG_RUSSIAN:
            core->wordlist.letter_count = wordlist_ru_letter_count;
            core->wordlist.word_count   = wordlist_ru_word_count;
            core->wordlist.hash         = wordlist_ru_hash;
            core->wordlist.list         = wordlist_ru;
            core->wordlist.lookup       = wordlist_ru_lookup;
            core->wordlist.offset       = wordlist_ru_offset;
            break;
    }
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
}

SDL_bool is_guess_allowed(const char* guess, game_t* core)
{
    Uint32       guess_hash;
    unsigned int offset_index;

    if (NULL == guess)
    {
        return SDL_FALSE;
    }

    guess_hash   = generate_hash(guess);
    offset_index = guess[0] - core->wordlist.lookup[0];

    if (0x0daa8447 == guess_hash) // NGAGE
    {
        return SDL_TRUE; // ;-)
    }

    if (offset_index >= 0 && offset_index < core->wordlist.letter_count)
    {
        int index;
        int start_index = core->wordlist.offset[offset_index];
        int end_index   = core->wordlist.offset[offset_index + 1];

        for (index = start_index; index < end_index; index +=1)
        {
            if (guess_hash == core->wordlist.hash[index])
            {
                return SDL_TRUE;
            }
        }
    }

    return SDL_FALSE;
}

void validate_current_guess(SDL_bool* is_won, game_t* core)
{
    Uint32       guess_hash;
    unsigned int offset_index;

    if (NULL == core)
    {
        return;
    }

    guess_hash   = generate_hash(core->current_guess);
    offset_index = core->current_guess[0] - core->wordlist.lookup[0];

    if (offset_index >= 0 && offset_index < core->wordlist.letter_count)
    {
        int    letter_index;
        int    answer_index;
        Uint32 answer_hash;
        int    start_index     = core->wordlist.offset[offset_index];
        int    end_index       = core->wordlist.offset[offset_index + 1];
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
