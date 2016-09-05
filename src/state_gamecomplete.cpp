#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"
#include "math.h"
#include "stdlib.h"

#include "debug.h"

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_mixer.h"
#include "SDL_ttf.h"
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

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Game.h"
#include "A4Object.h"
#include "A4Character.h"
#include "A4EngineApp.h"


int A4EngineApp::gamecomplete_cycle(KEYBOARDSTATE *k)
{
    if (m_state_cycle==0) {
        BInterface::reset();
        // create game complete screen
        BInterface::push();
        char *tmp = m_game->getGameEnding(m_game->getGameCompleteEndingID());
        if (tmp==0) {
            output_debug_message("Cannot find text for game ending with id '%s'!\n", m_game->getGameCompleteEndingID());
            exit(0);
        }
        BInterface::add_element(new BTextFrame(tmp, false, m_font8, 70, 10, 500, 380));
        BInterface::add_element(new BButton("Keep Playing", m_font16, 200, 400, 240, 30, 1));
        BInterface::add_element(new BButton("Quit", m_font16, 200, 440, 240, 30, 2));
    }

    int ID = BInterface::update_state(&m_mouse_clicks, k);
    switch(ID) {
        case 1:// keep playing
            BInterface::reset();
            m_game->setGameComplete(false, 0);
            return A4ENGINE_STATE_GAME;
            break;
        case 2:	// quit:
            BInterface::reset();
            m_game->setGameComplete(false, 0);
            m_ingame_menu = 0;
            return A4ENGINE_STATE_INTRO;
    }

    return A4ENGINE_STATE_GAMECOMPLETE;
}


void A4EngineApp::gamecomplete_draw(void)
{
    m_game->draw(m_screen_dx,m_screen_dy);
    
    if (m_cycles_per_frame>1) {
        if ((m_state_cycle%16)<8) {
            char tmp[80];
            sprintf(tmp,"x%i",m_cycles_per_frame);
            BInterface::print_left(tmp,m_font16,8,m_screen_dy-(16+4*10));
        }
    }
    
    BInterface::draw();
}
