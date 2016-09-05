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
#include "A4Scroll.h"
#include "A4Character.h"


A4Scroll::A4Scroll(Symbol *name, Sort *sort, int spell, int gold, Animation *a) : A4Item(name, sort)
{
	m_spell = spell;
	m_gold = gold;
	m_animations[A4_ANIMATION_IDLE] = a;
    setUsable(true);
}


A4Scroll::~A4Scroll()
{
}


void A4Scroll::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);

    if (a_event == A4_EVENT_USE && otherCharacter!=0) {
        if  (otherCharacter->getMaxMp()>0) {
            if (otherCharacter->knowsSpell(m_spell)) {
                game->addMessage("I already know that spell...");
            } else {
                otherCharacter->learnSpell(m_spell);
                otherCharacter->removeFromInventory(this);
                game->requestDeletion(this);
            }
        } else {
            game->addMessage("I can't decypher its content...");
        }
    }
}


bool A4Scroll::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"spell")==0) {
        m_spell = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4Scroll::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);

    saveObjectAttributeToXML(w,"spell",m_spell);    
}

