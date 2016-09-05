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
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
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
#include "A4AICharacter.h"
#include "AIMemory.h"

#include "A4Behavior.h"
#include "BFleeUnfriendly.h"  

BFleeUnfriendly::BFleeUnfriendly(int priority) : A4Behavior(priority)
{
    m_scaredActivation = 100;
}

A4CharacterCommand *BFleeUnfriendly::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    A4AI *ai = a_character->getAI();
    for(int i = 0;i<ai->m_n_unfriendly;i++) {
        WME *w = ai->m_unfriendly_cache[i];
        ai->addShortTermWME(new WME(A4Game::emotionNames[A4_EMOTION_SCARED],
									WMEParameter(a_character->getID()), WME_PARAMETER_INTEGER,
                                    w->getParameter(0), WME_PARAMETER_INTEGER,
                                    m_scaredActivation));
        ai->addPFTargetWME(w, a_game, A4CHARACTER_COMMAND_IDLE, m_priority, true);
    }
    return 0;
}


void BFleeUnfriendly::toJSString(char *buffer)
{
    sprintf(buffer,"BFleeUnfriendly()");
}


void BFleeUnfriendly::saveToXML(XMLwriter *w)
{
    w->openTag("BFleeUnfriendly");
    w->closeTag("BFleeUnfriendly");
}

