#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#endif

#include "assert.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <string>
#include <vector>

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_image.h"
#else
#include "SDL_image.h"
#endif

#include "SDLauxiliar.h"
#include "auxiliar.h"

#include "BitmapFont.h"

#define CHARACTERS_PER_FILE     128

BitmapFont::BitmapFont(const char *file)
{
    sfc=IMG_Load(file);
    assert(sfc!=0);
    dy = sfc->h;
    dx = sfc->w/CHARACTERS_PER_FILE;
}


BitmapFont::~BitmapFont()
{
    SDL_FreeSurface(sfc);
    sfc = 0;
}

int BitmapFont::getTextWidth(const char *text)
{
    return (int)strlen(text)*dx;
}

SDL_Surface *BitmapFont::render(const char *text)
{
    int l = UTF8StringLength(text);
    
    SDL_Surface *sfc2 = SDL_CreateRGBSurface(SDL_SWSURFACE,l*dx,dy,32,RMASK,GMASK,BMASK,AMASK);
    //    assert(sfc2!=0);
    for(int i = 0,j=0;text[i]!=0;) {
        SDL_Rect r1,r2;
        int c = UTF8ToExtendedASCII(text,i); // this already increments 'i'
        r1.x=dx*c;
        r1.y=0;
        r1.w=dx;
        r1.h=dy;
        r2.x=dx*j;
        r2.y=0;
        r2.w=dx;
        r2.h=dy;
        //        SDL_SetSurfaceAlphaMod(sfc, 0);
        SDL_BlitSurface(sfc,&r1,sfc2,&r2);
        j++;
    }
    
    return sfc2;
}

