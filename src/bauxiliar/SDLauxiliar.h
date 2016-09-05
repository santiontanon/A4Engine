#if SDL_BYTEORDER == SDL_BIG_ENDIAN
// PPC values:
#define RMASK  0xff000000
#define GMASK  0x00ff0000
#define BMASK  0x0000ff00
#define AMASK  0x000000ff
#define AOFFSET 3
#define BOFFSET 2
#define GOFFSET 1
#define ROFFSET 0

#else
// Intel values:
#define RMASK  0x000000ff
#define GMASK  0x0000ff00
#define BMASK  0x00ff0000
#define AMASK  0xff000000 
#define AOFFSET 3
#define BOFFSET 2
#define GOFFSET 1
#define ROFFSET 0

#endif

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
Uint32 getpixel(SDL_Surface *surface, int x, int y);
