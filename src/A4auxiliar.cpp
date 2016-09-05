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
#include <string>
#include <vector>

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_opengl.h"
#else
#include "SDL_ttf.h"
#include "SDL_mixer.h"
	#ifdef __APPLE__
		#include "OpenGL/gl.h"
		#include "OpenGL/glu.h"
	#else
		#include "GL/gl.h"
		#include "GL/glu.h"
	#endif
#endif
#include <vector>
#include "A4auxiliar.h"


std::vector<char *> splitByLines(char *initial_text)
{
    return splitStringBy(initial_text, '\n');
}


std::vector<char *> splitStringBy(char *str, char separator)
{
    std::vector<char *> l;
    int last = 0, i = 0;
    for(;str[i]!=0;i++) {
        if (str[i]==separator) {
            if (i>last) {
                char *tmp = new char[i-last+1];
                int j = 0;
                for(;j<i-last;j++) tmp[j] = str[j+last];
                tmp[j] = 0;
                l.push_back(tmp);
            }
            last = i+1;
        }
    }
    if (i>last) {
        char *tmp = new char[i-last+1];
        int j = 0;
        for(;j<i-last;j++) tmp[j] = str[j+last];
        tmp[j] = 0;
        l.push_back(tmp);
    }
    return l;
}

bool fileExists(const char *path)
{
    FILE *fp = fopen(path,"r+");
    if (fp==0) return false;
    fclose(fp);
    return true;
}


bool fileExists(const char *path, const char *file)
{
    char tmp[2048];
    sprintf(tmp,"%s/%s",path,file);
    return fileExists(tmp);
}

