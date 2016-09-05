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
#include "BCurious.h"


BCurious::BCurious(bool pickup, Sort *sort, int activation, int priority) : A4Behavior(priority)
{
    m_pickup = pickup;
    m_sort = sort;
    m_activation = activation;
    m_lastMemotyTimeChecked = -1;
    
    m_pattern1 = new WME(A4AI::s_object_symbol,
                           WMEParameter(m_sort), WME_PARAMETER_SORT,
                           WMEParameter(0), WME_PARAMETER_WILDCARD,
                           WMEParameter(0), WME_PARAMETER_WILDCARD,
                           WMEParameter(0), WME_PARAMETER_WILDCARD,
                           WMEParameter(0), WME_PARAMETER_WILDCARD,
                           WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
    m_pattern2 = new WME(AIMemory::s_is_a_symbol,
                            WMEParameter(0), WME_PARAMETER_INTEGER,
                            WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
    m_pattern3 = new WME(A4AI::s_object_symbol,
                         WMEParameter(0), WME_PARAMETER_INTEGER,
                         WMEParameter(0), WME_PARAMETER_WILDCARD,
                         WMEParameter(0), WME_PARAMETER_WILDCARD,
                         WMEParameter(0), WME_PARAMETER_WILDCARD,
                         WMEParameter(0), WME_PARAMETER_WILDCARD,
                         WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
}


BCurious::~BCurious()
{
    delete m_pattern1;
    m_pattern1 = 0;
    delete m_pattern2;
    m_pattern2 = 0;
    delete m_pattern3;
    m_pattern3 = 0;
}


A4CharacterCommand *BCurious::execute(A4AICharacter *a_character, class A4Game *a_game)
{
    A4AI *ai = a_character->getAI();
    AIMemory *m = ai->getMemory();
/*
    WME *pattern = new WME(A4AI::s_object_symbol,
                           {.m_sort = m_sort}, WME_PARAMETER_SORT,
                           {}, WME_PARAMETER_WILDCARD,
                           {}, WME_PARAMETER_WILDCARD,
                           {}, WME_PARAMETER_WILDCARD,
                           {}, WME_PARAMETER_WILDCARD,
                           {}, WME_PARAMETER_WILDCARD, 0);
    WME *pattern2 = new WME(AIMemory::s_is_a_symbol,
                            {.m_integer = 0}, WME_PARAMETER_INTEGER,
                            {}, WME_PARAMETER_WILDCARD, 0);
*/
    std::vector<WME *> *l = m->retrieveRelativeSubsumption(m_pattern1);
    for(WME *w:*l) {
        if (w->getParameterType(0)!=WME_PARAMETER_INTEGER) continue;
        // we are not curious about ourselves:
        if (w->getParameter(0).m_integer==a_character->getID()) continue;

        // determine how long have we known the object by checking for how long has the "isA" wme been in memory:
		m_pattern2->setParameter(0, WMEParameter(w->getParameter(0).m_integer));
        WME *w2 = m->retrieveFirstBySubsumption(m_pattern2);
        if (w2==0) continue;
        int activation = m_activation - (m_lastMemotyTimeChecked - w2->getStartTime());
        if (activation<=0) continue;
        ai->addShortTermWME(new WME(A4Game::emotionNames[A4_EMOTION_CURIOUS],
									WMEParameter(a_character->getID()), WME_PARAMETER_INTEGER,
                                    w->getParameter(0), w->getParameterType(0),
                                    activation));
    }
    delete l;
//    delete pattern;
//    delete pattern2;
    m_lastMemotyTimeChecked = m->getCycle();

    l = m->retrieveByFunctor(A4Game::emotionNames[A4_EMOTION_CURIOUS]);
    for(WME *w:*l) {
        /*
        WME *pattern2 = new WME(A4AI::s_object_symbol,
                               w->getParameter(0), WME_PARAMETER_INTEGER,
                               {}, WME_PARAMETER_WILDCARD,
                               {}, WME_PARAMETER_WILDCARD,
                               {}, WME_PARAMETER_WILDCARD,
                               {}, WME_PARAMETER_WILDCARD,
                               {}, WME_PARAMETER_WILDCARD, 0);
         */
        m_pattern3->setParameter(0, w->getParameter(1));
        std::vector<WME *> *l2 = m->retrieveSubsumption(m_pattern3);
        for(WME *w2:*l2) {
            if (m_pickup) {
                ai->addPFTargetWME(w2, a_game, A4CHARACTER_COMMAND_TAKE, m_priority, false);
            } else {
                ai->addPFTargetWME(w2, a_game, A4CHARACTER_COMMAND_IDLE, m_priority, false);
            }
        }
//        delete pattern2;
        delete l2;
    }
    delete l;
    return 0;
}



void BCurious::toJSString(char *buffer)
{
    sprintf(buffer,"BCurious(%s, %s, %i)", m_pickup ? "true":"false", m_sort->getName()->get(),m_activation);
}


void BCurious::saveToXML(XMLwriter *w)
{
    w->openTag("BCurious");
    w->setAttribute("pickup", m_pickup ? "true":"false");
    w->setAttribute("sort", m_sort->getName()->get());
    w->setAttribute("time", m_activation);
    w->closeTag("BCurious");
}
