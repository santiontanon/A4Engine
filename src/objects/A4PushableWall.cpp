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
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_mixer.h"
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
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4PushableWall.h"
#include "A4Character.h"

A4PushableWall::A4PushableWall(Sort *sort, Animation *a) : A4Object(0, sort)
{
    m_name = new Symbol("pushable-wall");
	m_animations[A4_ANIMATION_IDLE] = a;
}


A4PushableWall::~A4PushableWall()
{
}


void A4PushableWall::event(int a_event, class A4Character *character, class A4Map *map, class A4Game *game)
{
	A4Object::event(a_event,character,map,game);

	if (a_event==A4_EVENT_INTERACT) {
		int d = character->getDirection();
		if (m_map->walkable(m_x+A4Game::direction_x_inc[d]*m_animations[m_currentAnimation]->getPixelWidth(),
							m_y+A4Game::direction_y_inc[d]*m_animations[m_currentAnimation]->getPixelHeight(),
							m_animations[m_currentAnimation]->getPixelWidth(),
							m_animations[m_currentAnimation]->getPixelHeight(),this)) {
			// it can be pushed!
			event(A4_EVENT_PUSH, character, map, game);
		}
	} else if (a_event==A4_EVENT_PUSH) {
		int d = character->getDirection();
		m_x+=A4Game::direction_x_inc[d]*m_animations[m_currentAnimation]->getPixelWidth();
		m_y+=A4Game::direction_y_inc[d]*m_animations[m_currentAnimation]->getPixelHeight();
        if (character!=0) {
            map->reevaluateVisibility();
        }
	}
}
