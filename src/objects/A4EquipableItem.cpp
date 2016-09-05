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

#include "A4Item.h"
#include "A4EquipableItem.h"

A4EquipableItem::A4EquipableItem(Symbol *name, Sort *sort, class Animation *a, int es, int attack, int defense, float magic, float movement, bool canChop, int gold) : A4Item(name, sort)
{
	m_animations[A4_ANIMATION_IDLE] = a;
	m_equipable_slot = es;
	m_attack = attack;
	m_defense = defense;
	m_magic = magic;
    m_movement = movement;
	m_canChop = canChop;
	m_gold = gold;
}


A4EquipableItem::~A4EquipableItem()
{
}


bool A4EquipableItem::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"equipableSlot")==0) {
        m_equipable_slot = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"attack")==0) {
        m_attack = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"defense")==0) {
        m_defense = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"magic")==0) {
        m_magic = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"movement")==0) {
        m_movement = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"canChop")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            m_canChop = true;
        } else {
            m_canChop = false;
        }
        return true;
    }
    return false;
}


void A4EquipableItem::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w,game);
    
    saveObjectAttributeToXML(w,"equipableSlot",m_equipable_slot);
    saveObjectAttributeToXML(w,"attack",m_attack);
    saveObjectAttributeToXML(w,"defense",m_defense);
    if (m_magic!=1.0f) saveObjectAttributeToXML(w,"magic",m_magic);
    if (m_movement!=1.0f) saveObjectAttributeToXML(w,"movement",m_movement);
    saveObjectAttributeToXML(w,"canChop",m_canChop);
}

