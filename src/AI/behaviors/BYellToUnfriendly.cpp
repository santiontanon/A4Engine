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
#include "BYellToUnfriendly.h"

BYellToUnfriendly::BYellToUnfriendly(char *message, int priority) : A4Behavior(priority)
{
    m_message = new char[strlen(message)+1];
    strcpy(m_message, message);
    m_angryActivation = 250;
}


BYellToUnfriendly::~BYellToUnfriendly() {
    if (m_message!=0) delete m_message;
    m_message = 0;
}


A4CharacterCommand *BYellToUnfriendly::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
//    A4Map *map = a_character->getMap();
    A4AI *ai = a_character->getAI();
    AIMemory *m = ai->getMemory();
    if (m->retrieveFirstByFunctor(A4Game::emotionNames[A4_EMOTION_ANGRY])!=0) return 0;
    if (ai->m_n_unfriendly>0) {
        WME *w = ai->m_unfriendly_cache[0];
        ai->addShortTermWME(new WME(A4Game::emotionNames[A4_EMOTION_ANGRY],
                                    w->getParameter(0), WME_PARAMETER_INTEGER,
                                    m_angryActivation));
        // yell:
        char *tmp = new char[strlen(m_message)+1];
        strcpy(tmp, m_message);
        return new A4CharacterCommand(A4CHARACTER_COMMAND_TALK_ANGRY, 0, 0,
                                      new SpeechAct(A4_TALK_PERFORMATIVE_INFORM, 0, tmp), m_priority);
    }
    return 0;
}


void BYellToUnfriendly::toJSString(char *buffer)
{
    sprintf(buffer,"BYellToUnfriendly(\"%s\")",m_message);
}


void BYellToUnfriendly::saveToXML(XMLwriter *w)
{
    w->openTag("BYellToUnfriendly");
    w->setAttribute("message", m_message);
    w->closeTag("BYellToUnfriendly");
}

