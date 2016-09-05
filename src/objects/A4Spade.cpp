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
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_mixer.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"
#endif
#endif
#include "SDLauxiliar.h"
#include "GLauxiliar.h"

#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "SFXManager.h"
#include "BitmapFont.h"
#include "Binterface.h"
#include "XMLparser.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4Item.h"
#include "A4Spade.h"
#include "A4Character.h"

A4Spade::A4Spade(Sort *sort, Animation *a, int gold) : A4Item(0, sort)
{
    m_name = new Symbol("Spade");
	m_animations[A4_ANIMATION_IDLE] = a;
	m_gold = gold;
    setUsable(true);
}


A4Spade::~A4Spade()
{
}


void A4Spade::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        A4Object *o = map->getBurrowedObject(otherCharacter->getX(), otherCharacter->getY(),
                                             otherCharacter->getPixelWidth(), otherCharacter->getPixelHeight());
        if (o==0) {
            game->addMessage("Nothing to dig here...");
        } else {
            o->setBurrowed(false);
        }
    }
}

