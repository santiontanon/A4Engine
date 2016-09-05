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
#include "A4Character.h"

#include "A4PressurePlate.h"

A4PressurePlate::A4PressurePlate(Sort *sort, Animation *pressed, Animation *released, int pr) : A4Object(0, sort)
{
    m_name = new Symbol("pressure-plate");
	m_pressurePlateState = false;
	m_animations[A4_ANIMATION_CLOSED] = pressed;
	m_animations[A4_ANIMATION_OPEN] = released;
	m_pressureRequired = pr;
	if (m_pressurePlateState) m_currentAnimation = A4_ANIMATION_CLOSED;
                         else m_currentAnimation = A4_ANIMATION_OPEN;
}


A4PressurePlate::~A4PressurePlate()
{
}


bool A4PressurePlate::cycle(A4Game *game)
{
	A4Object::cycle(game);

	std::vector<A4Object *> *l = m_map->getAllObjectCollisions(this);
	A4Object *heaviest = 0;
	int pressure = 0;

	for(A4Object *o:*l) {
		if (pressure<TRIGGER_PRESSURE_ITEM) {
			pressure = TRIGGER_PRESSURE_ITEM;	// if there is an object, at least there is pressure 1
			heaviest = o;
		}
		if (pressure<TRIGGER_PRESSURE_HEAVY_ITEM && o->isHeavy()) {
			pressure = TRIGGER_PRESSURE_HEAVY_ITEM;
			heaviest = o;
		}
		if (pressure<TRIGGER_PRESSURE_PLAYER && o->isPlayer()) {
			pressure = TRIGGER_PRESSURE_PLAYER;
			heaviest = o;
		}
	}

	if (m_pressurePlateState) {
		if (pressure>=m_pressureRequired) {
			// nothing to do, keep pressed
		} else {
			// release
			m_pressurePlateState = false;
			event(A4_EVENT_DEACTIVATE,0, m_map, game);
			event(A4_EVENT_USE,0, m_map, game);
		    m_currentAnimation = A4_ANIMATION_OPEN;	
		}
	} else {
		if (heaviest!=0 && pressure>=m_pressureRequired) {
			// press!
			m_pressurePlateState = true;
			if (heaviest->isPlayer()) {
				event(A4_EVENT_ACTIVATE,(A4Character *)heaviest, m_map, game);
				event(A4_EVENT_USE,(A4Character *)heaviest, m_map, game);
			} else {
				event(A4_EVENT_ACTIVATE,0, m_map, game);
				event(A4_EVENT_USE,0, m_map, game);
			}
		    m_currentAnimation = A4_ANIMATION_CLOSED;	
		} else {
			// nothing to do, keep released
		}
	}

	l->clear();
	delete l;

	return true;
}

bool A4PressurePlate::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"pressurePlateState")==0) {
        m_pressurePlateState = false;
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) m_pressurePlateState = true;
        return true;
    } else if (strcmp(a_name,"pressureRequired")==0) {
        m_pressureRequired = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}

void A4PressurePlate::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"pressureRequired",m_pressureRequired);
    saveObjectAttributeToXML(w,"pressurePlateState",m_pressurePlateState);
}

