#ifndef __BRAIN_BITMAP_FONT
#define __BRAIN_BITMAP_FONT

class BitmapFont {
public:
    BitmapFont(const char *file);
    ~BitmapFont();
    
    SDL_Surface *render(const char *text);
    
    int getHeight(void) {
        return dy;
    }
    int getWidth(void) {return dx;}
    int getTextWidth(const char *text);
    
private:
    int dx,dy;
    SDL_Surface *sfc;
};

#endif