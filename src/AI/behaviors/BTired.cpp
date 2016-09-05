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
#include "BTired.h"

BTired::BTired(int timeWalking, int restTime, int priority) : A4Behavior(priority)
{
    m_timeWalkingUntilTired = timeWalking;
    m_restTime = restTime;
    m_currentTimeWalking = 0;
}

A4CharacterCommand *BTired::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    A4AI *ai = a_character->getAI();
    AIMemory *m = ai->getMemory();

    if (m->WMEwithFunctorExists(A4Game::emotionNames[A4_EMOTION_TIRED])) {
        return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, m_priority);
    }

    if (a_character->getPreviousState() == A4CHARACTER_STATE_WALKING) {
        m_currentTimeWalking++;
    } else {
        if (m_currentTimeWalking>0) m_currentTimeWalking--;
    }
    if (m_currentTimeWalking>m_timeWalkingUntilTired) {
        ai->addShortTermWME(new WME(A4Game::emotionNames[A4_EMOTION_TIRED],
                                    WMEParameter(a_character->getID()), WME_PARAMETER_INTEGER,
                                    m_restTime));
        m_currentTimeWalking = 0;
    }
    return 0;
}


void BTired::toJSString(char *buffer)
{
    sprintf(buffer,"BTired(%i, %i)", m_timeWalkingUntilTired, m_restTime);
}


void BTired::saveToXML(XMLwriter *w)
{
    w->openTag("BTired");
    w->setAttribute("timeWalking", m_timeWalkingUntilTired);
    w->setAttribute("restTime", m_restTime);
    w->closeTag("BTired");
}


