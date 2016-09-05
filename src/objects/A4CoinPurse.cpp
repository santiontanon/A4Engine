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
#include "A4CoinPurse.h"
#include "A4Character.h"

A4CoinPurse::A4CoinPurse(Sort *sort, int gold, Animation *a, bool createScripts) : A4Item(0, sort)
{
    m_name = new Symbol("CoinPurse");
	m_gold = gold;
	m_animations[A4_ANIMATION_IDLE] = a;

    /*
    if (createScripts) {
        m_event_scripts[A4_EVENT_PICKUP].push_back(new A4EventRule(A4_EVENT_PICKUP, new A4Script(A4_SCRIPT_GAINGOLDOTHER, gold), false));
        m_event_scripts[A4_EVENT_PICKUP].push_back(new A4EventRule(A4_EVENT_PICKUP, new A4Script(A4_SCRIPT_LOSEITEM, "CoinPurse"), false));
    }*/
    setUsable(true);
}


A4CoinPurse::~A4CoinPurse()
{
}


void A4CoinPurse::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);

    if (a_event == A4_EVENT_USE) {
        otherCharacter->setGold(otherCharacter->getGold()+m_gold);
        otherCharacter->removeFromInventory(this);
        map->removeObject(this);
        game->requestDeletion(this);
    } else if (a_event == A4_EVENT_PICKUP) {
        A4Script *s = new A4Script(A4_SCRIPT_GAINGOLDOTHER, m_gold);
        s->execute(this, map, game, otherCharacter);
        delete s;
        s = new A4Script(A4_SCRIPT_LOSEITEM, "CoinPurse");
        s->execute(this, map, game, otherCharacter);
        delete s;
    }
}

