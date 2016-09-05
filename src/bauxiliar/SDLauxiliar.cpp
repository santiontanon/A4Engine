#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#endif

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include "SDL.h"


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    SDL_Rect clip;
    int bpp = surface->format->BytesPerPixel;

    SDL_GetClipRect(surface,&clip);

    if (x<clip.x || x>=clip.x+clip.w ||
        y<clip.y || y>=clip.y+clip.h) return;

    if (x<0 || x>=surface->w ||
        y<0 || y>=surface->h) return;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = (Uint8)pixel;
        break;

    case 2:
        *(Uint16 *)p = (Uint16)pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = Uint8( (pixel >> 16) & 0xff );
            p[1] = Uint8( (pixel >> 8) & 0xff );
            p[2] = Uint8( pixel & 0xff );
        } else {
            p[0] = Uint8( pixel & 0xff );
            p[1] = Uint8( (pixel >> 8) & 0xff );
            p[2] = Uint8( (pixel >> 16) & 0xff );
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }

}


Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

    if (x<0 || x>=surface->w ||
        y<0 || y>=surface->h) return 0;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;
    }
}