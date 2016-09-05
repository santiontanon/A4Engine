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
#include "BWander.h"

BWander::BWander(double forwardFactor, double pStop, int stopTime, int priority) : A4Behavior(priority)
{
    m_forwardFactor = forwardFactor;
    m_pStop = pStop;
    m_stopTime = stopTime;
    m_state = 0;
    m_currentDirection = A4_DIRECTION_NONE;
}


A4CharacterCommand *BWander::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    int movement_slack = 0; // this reduces the computational burden of this behavior
    if (m_state == 0) {
        // stop?
        if (((double)rand() / RAND_MAX) < m_pStop) {
            // stop:
            m_state = m_stopTime;
            m_currentDirection = A4_DIRECTION_NONE;
        } else {
            // choose one of the possible directions at random:
            int directions[4];
            double weights[4];
            double totalWeight = 0;
            int nDirections = 0;
            for(int i = 0;i<A4_NDIRECTIONS;i++) {
                if (a_character->canMove(i,movement_slack) != INT_MAX) {
                    directions[nDirections] = i;
                    weights[nDirections] = (i == m_currentDirection ? m_forwardFactor:1.0);
                    totalWeight += weights[nDirections];
                    nDirections ++;
                }
            }
            if (nDirections>0) {
                double r = totalWeight * ((double)rand() / RAND_MAX);
                int selected = -1;
                for(int i = 0;i<nDirections;i++) {
                    if (r < weights[i]) {
                        selected = i;
                        break;
                    }
                    r -= weights[i];
                }
                if (selected==-1) selected = nDirections-1;
                m_state = a_character->getWalkSpeed();
                m_currentDirection = directions[selected];
            } else {
                // nowhere to go, so, stop:
                m_state = m_stopTime;
                m_currentDirection = A4_DIRECTION_NONE;
            }
        }
    }

    if (m_state>0) m_state--;
    return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0 , m_currentDirection, 0, m_priority);
}


void BWander::toJSString(char *buffer)
{
    sprintf(buffer,"BWander(%g, %g, %i)",m_forwardFactor, m_pStop, m_stopTime);
}


void BWander::saveToXML(XMLwriter *w)
{
    w->openTag("BWander");
    w->setAttribute("forwardFactor", m_forwardFactor);
    w->setAttribute("pStop", m_pStop);
    w->setAttribute("stopTime", m_stopTime);    
    w->closeTag("BWander");
}


