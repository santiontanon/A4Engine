//
//  BFollowFromWME.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 6/28/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

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

#include "BFollowFromWME.h"


BFollowFromWME::BFollowFromWME(int radius, int priority) : A4Behavior(priority)
{
    m_radius_sq = radius*radius;
    m_inner_radius_sq = (radius-32)*(radius-32);
    follow_symbol = new Symbol("follow");
}


BFollowFromWME::~BFollowFromWME() {
    delete follow_symbol;
    follow_symbol = 0;
}


A4CharacterCommand *BFollowFromWME::execute(A4AICharacter *a_character, class A4Game *a_game)
{
    A4AI *ai = a_character->getAI();
    AIMemory *m = ai->getMemory();
    std::vector<WME *> *l1 = m->retrieveByFunctor(follow_symbol);
    for(WME *fwme:*l1) {
        WME *pattern = new WME(A4AI::s_object_symbol,
                               fwme->getParameter(0), fwme->getParameterType(0),
                               WMEParameter(0), WME_PARAMETER_WILDCARD,
                               WMEParameter(0), WME_PARAMETER_WILDCARD,
                               WMEParameter(0), WME_PARAMETER_WILDCARD,
                               WMEParameter(0), WME_PARAMETER_WILDCARD,
                               WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
        std::vector<WME *> *l2 = m->retrieveRelativeSubsumption(pattern);
        for(WME *w:*l2) {
            if (w->getParameter(5).m_symbol->cmp(a_character->getMap()->getNameSymbol())) {
                if (w->getParameterType(1) == WME_PARAMETER_INTEGER &&
                    w->getParameterType(2) == WME_PARAMETER_INTEGER &&
                    w->getParameterType(3) == WME_PARAMETER_INTEGER &&
                    w->getParameterType(4) == WME_PARAMETER_INTEGER) {
                    // this assumes that the radius is from the center of the location:
                    int cx = (w->getParameter(1).m_integer + w->getParameter(3).m_integer)/2;
                    int cy = (w->getParameter(2).m_integer + w->getParameter(4).m_integer)/2;
                    int dx = cx - (a_character->getX() + a_character->getPixelWidth()/2);
                    int dy = cy - (a_character->getY() + a_character->getPixelHeight()/2);
                    int d_sq = dx*dx + dy*dy;
                    if (d_sq > m_radius_sq) {
                        ai->addPFTargetWME(w, a_game, A4CHARACTER_COMMAND_IDLE, m_priority, false);
                    } else if (d_sq < m_inner_radius_sq) {
                        ai->addPFTargetWME(w, a_game, A4CHARACTER_COMMAND_IDLE, m_priority, true);
                    } else {
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, m_priority);
                    }
                }
            } else {
                ai->addPFTargetWME(w, a_game, A4CHARACTER_COMMAND_IDLE, m_priority, false);
            }
        }
        delete l2;
    }
    delete l1;
    return 0;
}


void BFollowFromWME::toJSString(char *buffer)
{
    sprintf(buffer,"BFollowFromWME(%i)",int(sqrt(m_radius_sq)));
}


void BFollowFromWME::saveToXML(XMLwriter *w)
{
    w->openTag("BFollowFromWME");
    w->setAttribute("radius", int(sqrt(m_radius_sq)));
    w->closeTag("BFollowFromWME");
}


