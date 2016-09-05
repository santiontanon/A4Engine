#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_mixer.h"
#else
#include "SDL_mixer.h"
#endif
#include "sound.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "assert.h"

#include "debug.h"


#define AUDIO_BUFFER 2048

bool sound_enabled = false;
Mix_Music *music_sound = 0;
int n_channels = -1;

bool Sound_initialization(void)
{
    if ( -1 == Sound_initialization(0, 0)) {
        return false;
    }
    return true;
}

int Sound_initialization(int nc, int nrc)
{
    int audio_rate = 44100;
    int audio_channels = 2;
    int audio_bufsize = AUDIO_BUFFER;
    Uint16 audio_format = AUDIO_S16;
    SDL_version compile_version;
    n_channels = 8;
    
    sound_enabled = true;
    
#ifdef __DEBUG_MESSAGES
    output_debug_message("Initializing SDL_mixer.\n");
#endif
    
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_bufsize)) {
        sound_enabled = false;
#ifdef __DEBUG_MESSAGES
        
        output_debug_message("Unable to open audio: %s\n", Mix_GetError());
        output_debug_message("Running the game without audio.\n");
#endif
        
        return -1;
    }
    
    Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);
#ifdef __DEBUG_MESSAGES
    
    output_debug_message("    Opened %d Hz %d bit %s, %d bytes audio buffer\n",
                         audio_rate, audio_format & 0xFF,
                         audio_channels > 1 ? "stereo" : "mono", audio_bufsize);
#endif
    
    MIX_VERSION (&compile_version);
#ifdef __DEBUG_MESSAGES
    
    output_debug_message("    Compiled with SDL_mixer version: %d.%d.%d\n",
                         compile_version.major,
                         compile_version.minor,
                         compile_version.patch);
    output_debug_message("    Running with SDL_mixer version: %d.%d.%d\n",
                         Mix_Linked_Version()->major,
                         Mix_Linked_Version()->minor,
                         Mix_Linked_Version()->patch);
#endif
    
    if (nc > 0) {
        n_channels = Mix_AllocateChannels(nc);
    }
    if (nrc > 0) {
        Mix_ReserveChannels(nrc);
    }
    return n_channels;
}
void Sound_release(void)
{
    if (sound_enabled) {
        Mix_CloseAudio();
    }
    sound_enabled = false;
}


/* a check to see if file is readable and greater than zero */
int file_check(char *fname)
{
    FILE *fp;
    
    if ((fp = fopen(fname, "r")) != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0 && ftell(fp) > 0) {
            fclose(fp);
            return true;
        }
        /* either the file could not be read (== -1) or size was zero (== 0) */
#ifdef __DEBUG_MESSAGES
        output_debug_message("ERROR in file_check(): the file %s is corrupted.\n", fname);
#endif
        
        fclose(fp);
        exit(1);
    }
    return false;
}

