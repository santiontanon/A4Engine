#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"
#include "math.h"
#include "stdlib.h"

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
#include "BitmapFont.h"
#include "Binterface.h"

#include "A4EngineApp.h"
#include "A4Game.h"


int A4EngineApp::intro_cycle(KEYBOARDSTATE *k)
{
	if (k->key_press(SDL_SCANCODE_ESCAPE)) return A4ENGINE_STATE_TITLESCREEN;
	if (k->key_press(SDL_SCANCODE_SPACE) ||
		k->key_press(SDL_SCANCODE_RETURN) ||
		!m_mouse_clicks.empty()) {
		if (!m_mouse_clicks.empty()) m_mouse_clicks.pop_back();
		if (m_state_cycle<350) m_state_cycle = 350;
					      else return A4ENGINE_STATE_TITLESCREEN;
	} // if
    if (m_game==0 && m_state_cycle>=350) return A4ENGINE_STATE_TITLESCREEN;
	if (m_state_cycle>650) return A4ENGINE_STATE_TITLESCREEN;
	return A4ENGINE_STATE_INTRO;
} /* A4EngineApp::intro_cycle */


void A4EngineApp::intro_draw(void)
{
	glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    float f1 = 1.0;
    float f2 = 1.0;

	if (m_state_cycle>=0 && m_state_cycle<300) {
		if (m_state_cycle>=0 && m_state_cycle<50) {
			f1=m_state_cycle/50.0f;
			f2=0.75f+(m_state_cycle/250.0f)*0.25f;
		}
		if (m_state_cycle>=50 && m_state_cycle<250) {
			f1 = 1.0;
			f2=0.75f+(m_state_cycle/250.0f)*0.25f;
		}
		if (m_state_cycle>=250 && m_state_cycle<300) {
			f1=(300-m_state_cycle)/50.0f;
			f2=0.75f+(m_state_cycle/250.0f)*0.25f;
		}
		GLTile *t = BInterface::get_text_tile("BRAIN GAMES",m_font32);
		GLTile *t2 = BInterface::get_text_tile("Santiago Ontanon (2016)",m_font16);
		t->set_hotspot(t->get_dx()/2,t->get_dy()+4);
		t2->set_hotspot(t2->get_dx()/2,-4);
		t->draw(1,1,1,f1,320,240,0,0,f2);
		t2->draw(1,1,1,f1,320,240,0,0,f2);
	} // if

	if (m_state_cycle>=350 && m_game!=0) {
		float f1=(m_state_cycle-350)/300.0f;
        if (m_game->getGameTitleImage()!=0) {
            char path[1024];
            sprintf(path,"%s/%s",m_game_path,m_game->getGameTitleImage());
            GLTile *ti = m_GLTM->get(path);
            if (ti!=0) {
                ti->set_hotspot(ti->get_dx()/2,ti->get_dy()/2);
                ti->draw(1,1,1,f1*f1,320,240,0,0,1);
            }
        }
        
		GLTile *t = BInterface::get_text_tile(m_game->getGameTitle(),m_font32);
		t->set_hotspot(t->get_dx()/2,t->get_dy()/2);
		t->draw(1,1,1,f1*f1,320,240*(1-f1)+120*f1,0,1.0f-f1*1.0f,4*(1-f1)+f1);
	} // if
} /* A4EngineApp::intro_draw */

