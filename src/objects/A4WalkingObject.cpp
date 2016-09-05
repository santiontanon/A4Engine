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

#include <algorithm>
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
#include "A4WalkingObject.h"

#include "A4ObjectFactory.h"

A4WalkingObject::A4WalkingObject(Symbol *name, Sort *sort) : A4Object(name, sort)
{
    m_currentAnimation = A4_ANIMATION_IDLE_RIGHT;
    
    m_walk_speed = 16;
    
    m_previous_state = A4CHARACTER_STATE_NONE;
    m_state = A4CHARACTER_STATE_IDLE;
    m_state_cycle = 0;
    m_previous_direction = A4_DIRECTION_NONE;
    m_direction = A4_DIRECTION_LEFT;
    
    for(int i = 0;i<A4_NDIRECTIONS;i++) {
        m_continuous_direction_command_timers[i] = 0;
        m_continuous_direction_command_max_movement[i] = 0;
        m_direction_command_received_this_cycle[i] = false;
    }
    
    m_walking_counter = 0;
}


A4WalkingObject::~A4WalkingObject()
{
}


bool A4WalkingObject::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"walk_speed")==0) {
        setWalkSpeed(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"direction")==0) {
        m_direction = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"previous_direction")==0) {
        m_previous_direction = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"state")==0) {
        m_state = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"previous_state")==0) {
        m_previous_state = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"state_cycle")==0) {
        m_state_cycle = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}



void A4WalkingObject::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w,game);
    
    saveObjectAttributeToXML(w,"walk_speed",m_walk_speed);
    saveObjectAttributeToXML(w,"direction",m_direction);
    saveObjectAttributeToXML(w,"previous_direction",m_previous_direction);
    saveObjectAttributeToXML(w,"state",m_state);
    saveObjectAttributeToXML(w,"previous_state",m_previous_state);
    saveObjectAttributeToXML(w,"state_cycle",m_state_cycle);    
}


