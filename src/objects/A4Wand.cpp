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
#include "A4Wand.h"
#include "A4Character.h"


A4Wand::A4Wand(Symbol *name, Sort *sort, int spell, int charges, bool disappear, int gold, Animation *a) : A4Item(name, sort)
{
    m_spell = spell;
    m_charges = charges;
    m_disappearWhenEmpty = disappear;
    m_gold = gold;
    m_animations[A4_ANIMATION_IDLE] = a;
    setUsable(true);
}


A4Wand::~A4Wand()
{
}


void A4Wand::eventWithInteger(int a_event, int value, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE && otherCharacter!=0) {
        // value is the direction of the scroll ("no direction" means, use in self):
        if (m_charges>0) {
            if (otherCharacter->castSpell(m_spell, value, game, true)) m_charges--;
            if (m_charges<=0 && m_disappearWhenEmpty) {
                otherCharacter->removeFromInventory(this);
                game->requestDeletion(this);
            }
        }
    }
}


bool A4Wand::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"spell")==0) {
        m_spell = atoi(attribute_xml->get_attribute("value"));;
        return true;
    } else if (strcmp(a_name,"charges")==0) {
        m_charges = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"disappearWhenEmpty")==0) {
        m_disappearWhenEmpty = false;
        char *tmp = attribute_xml->get_attribute("value");
        if (tmp!=0 && strcmp(tmp,"true")==0) m_disappearWhenEmpty = true;
        return true;
    }
    return false;
}


void A4Wand::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"spell",m_spell);
    saveObjectAttributeToXML(w,"charges",m_charges);
    saveObjectAttributeToXML(w,"disappearWhenEmpty",m_disappearWhenEmpty);
}

