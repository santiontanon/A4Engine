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
#include "BLeftRight.h"

BLeftRight::BLeftRight(int priority) : A4Behavior(priority)
{
    m_state = 0;
}


A4CharacterCommand *BLeftRight::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    int movement_slack = a_character->getPixelWidth()*A4_MOVEMENT_SLACK;
    switch(m_state) {
        case 0: // left
            if (a_character->canMove(A4_DIRECTION_LEFT,movement_slack)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0, A4_DIRECTION_LEFT, 0, m_priority);
            } else {
                m_state = 1;
                if (a_character->canMove(A4_DIRECTION_RIGHT,movement_slack)!=INT_MAX) {
                    return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0, A4_DIRECTION_RIGHT, 0, m_priority);
                }
            }
            break;
        case 1: // right
            if (a_character->canMove(A4_DIRECTION_RIGHT,movement_slack)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0, A4_DIRECTION_RIGHT, 0, m_priority);
            } else {
                m_state = 0;
                if (a_character->canMove(A4_DIRECTION_LEFT,movement_slack)!=INT_MAX) {
                    return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, 0, A4_DIRECTION_LEFT, 0, m_priority);
                }
            }
            break;
    }
    return 0;
}


void BLeftRight::toJSString(char *buffer)
{
    sprintf(buffer,"BLeftRight()");
}


void BLeftRight::saveToXML(XMLwriter *w)
{
    w->openTag("BLeftRight");
    w->closeTag("BLeftRight");
}

