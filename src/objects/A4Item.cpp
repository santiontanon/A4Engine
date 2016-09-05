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

#include "A4Item.h"

A4Item::A4Item(Symbol *name, Sort *sort) : A4Object(name, sort)
{
	m_takeable = true;
	m_equipable = false;
	m_useUponTake = false;
	m_burrowed = false;
	m_attackBonus = 0;
	m_defenseBonus = 0;
	m_magicMultiplier = 1.0f;
}


A4Item::~A4Item()
{
}


bool A4Item::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"equipable")==0) {
        m_equipable = false;
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) m_equipable = true;
        return true;
    } else if (strcmp(a_name,"useUponTake")==0) {
        m_useUponTake = false;
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) m_useUponTake = true;
        return true;
    } else if (strcmp(a_name,"attackBonus")==0) {
        m_attackBonus = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"defenseBonus")==0) {
        m_defenseBonus = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"magicMultiplier")==0) {
        m_magicMultiplier = atof(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4Item::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"equipable",m_equipable);
    saveObjectAttributeToXML(w,"useUponTake",m_useUponTake);
    saveObjectAttributeToXML(w,"attackBonus",m_attackBonus);
    saveObjectAttributeToXML(w,"defenseBonus",m_defenseBonus);
    saveObjectAttributeToXML(w,"magicMultiplier",m_magicMultiplier);
}


