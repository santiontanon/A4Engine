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
#include "XMLwriter.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4Lever.h"

A4Lever::A4Lever(Sort *sort, Symbol *ID, bool state, class Animation *a_closed, Animation *a_open) : A4Object(0, sort)
{
    m_name = new Symbol("lever");
	m_leverID = ID;
	m_leverState = state;
	m_animations[A4_ANIMATION_CLOSED] = a_closed;
	m_animations[A4_ANIMATION_OPEN] = a_open;
	if (m_leverState) m_currentAnimation = A4_ANIMATION_CLOSED;
                 else m_currentAnimation = A4_ANIMATION_OPEN;

	m_usable = true;
}


A4Lever::~A4Lever()
{
    delete m_leverID;
    m_leverID = 0;
}

void A4Lever::event(int event_type, class A4Character *character, class A4Map *map, class A4Game *game)
{
	A4Object::event(event_type, character, map, game);
	if (event_type == A4_EVENT_USE) {
        A4Script *s = new A4Script(A4_SCRIPT_OPENDOORS, m_leverID->get());
        s->execute(this, map, game, character);
        delete s;
        
		m_leverState = (m_leverState ? false:true);
		if (m_leverState) {
			event(A4_EVENT_ACTIVATE,character, m_map, game);
		} else {
			event(A4_EVENT_DEACTIVATE,character, m_map, game);
		}
		if (m_leverState) m_currentAnimation = A4_ANIMATION_CLOSED;
                     else m_currentAnimation = A4_ANIMATION_OPEN;
	}	
}


bool A4Lever::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"leverID")==0) {
        if (m_leverID!=0) delete m_leverID;
        m_leverID = new Symbol(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"leverState")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            m_leverState = true;
        } else {
            m_leverState = false;
        }
        if (m_leverState) m_currentAnimation = A4_ANIMATION_CLOSED;
                     else m_currentAnimation = A4_ANIMATION_OPEN;
        return true;
    }
    return false;
}


void A4Lever::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"leverID",m_leverID->get());
    saveObjectAttributeToXML(w,"leverState",m_leverState);
}


