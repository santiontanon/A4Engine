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

#include "A4Behavior.h"
#include "BWanderInCircles.h"

BWanderInCircles::BWanderInCircles(int diameter, int priority) : A4Behavior(priority)
{
    m_diameter = diameter;
    m_state = 0;
    m_currentDirection = A4_DIRECTION_NONE;
}


A4CharacterCommand *BWanderInCircles::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    if (m_state == 0) {
        m_currentDirection++;
        if (m_currentDirection>=A4_NDIRECTIONS) m_currentDirection = A4_DIRECTION_LEFT;
        m_state = a_character->getWalkSpeed()*m_diameter;
    }

    if (m_state>0) m_state--;
    return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0, m_currentDirection, 0, m_priority);
}


void BWanderInCircles::toJSString(char *buffer)
{
    sprintf(buffer,"BWanderInCircles(%i)",m_diameter);
}


void BWanderInCircles::saveToXML(XMLwriter *w)
{
    w->openTag("BWanderInCircles");
    w->setAttribute("diameter", m_diameter);
    w->closeTag("BWanderInCircles");
}

