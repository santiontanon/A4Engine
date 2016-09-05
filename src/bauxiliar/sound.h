#ifndef __BRAIN_SDL_SOUND
#define __BRAIN_SDL_SOUND

bool Sound_initialization(void);
int Sound_initialization(int nc, int nrc);
void Sound_release(void);

/*
 int Sound_play(Mix_Chunk s);
 int Sound_play(Mix_Chunk *s, int volume);
 int Sound_play(Mix_Chunk *s, int volume, int distance);
 int Sound_play(Mix_Chunk *s, int volume, int angle, int distance);
 
 int Sound_play_continuous(Mix_Chunk *s);
 int Sound_play_continuous(Mix_Chunk *s, int volume);
 int Sound_play_continuous(Mix_Chunk *s, int volume, int distance);
 int Sound_play_continuous(Mix_Chunk *s, int volume, int angle, int distance);
 
 int Sound_play_ch(Mix_Chunk *s, int channel, int volume);
 int Sound_play_ch(Mix_Chunk *s, int channel, int volume, int distance);
 int Sound_play_ch(Mix_Chunk *s, int channel, int volume, int angle, int distance);
 */

#endif

