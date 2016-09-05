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
#include "A4Potion.h"
#include "A4Character.h"

A4Potion::A4Potion(Symbol *name, Sort *sort, Animation *a) : A4Item(name, sort)
{
	m_animations[A4_ANIMATION_IDLE] = a;
    setUsable(true);
}


A4HPPotion::A4HPPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
	m_hp = v;
    m_gold = 5 + m_hp*2;
}


void A4HPPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        if (otherCharacter->getHp() < otherCharacter->getMaxHp()) {
            otherCharacter->recoverHp(m_hp);
            otherCharacter->removeFromInventory(this);
            game->requestDeletion(this);
        } else {
            if (game->getCurrentPlayer()==otherCharacter) game->addMessage("Nah, I'm not hurt. That would be a waste!");
        }
    }
}


bool A4HPPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"hp")==0) {
        m_hp = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4HPPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"hp",m_hp);
}


// ----------------------------------------------------------------


A4MPPotion::A4MPPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
	m_mp = v;
    m_gold = 5 + m_mp*2;
}


void A4MPPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        if (otherCharacter->getMp() < otherCharacter->getMaxMp()) {
            otherCharacter->recoverMp(m_mp);
            otherCharacter->removeFromInventory(this);
            game->requestDeletion(this);
        } else {
            if (game->getCurrentPlayer()==otherCharacter) game->addMessage("Nah, that would be a waste!");
        }
    }
}


bool A4MPPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"mp")==0) {
        m_mp = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4MPPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"mp",m_mp);
}


// ----------------------------------------------------------------


A4XPPotion::A4XPPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
	m_xp = v;
    m_gold = 10 + m_xp*4;
}


void A4XPPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        otherCharacter->gainXp(m_xp);
        otherCharacter->removeFromInventory(this);
        game->requestDeletion(this);
    }
}


bool A4XPPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"xp")==0) {
        m_xp = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4XPPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"xp",m_xp);
}


// ----------------------------------------------------------------


A4StrengthPotion::A4StrengthPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
    m_strength = v;
    m_gold = 100 + m_strength*100;
}


void A4StrengthPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        otherCharacter->setAttack(otherCharacter->getBaseAttack()+m_strength);
        otherCharacter->removeFromInventory(this);
        game->requestDeletion(this);
    }
}


bool A4StrengthPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"strength")==0) {
        m_strength = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4StrengthPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"strength",m_strength);
}


// ----------------------------------------------------------------


A4ConstitutionPotion::A4ConstitutionPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
    m_constitution = v;
    m_gold = 50 + m_constitution*50;
}



void A4ConstitutionPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        otherCharacter->setDefense(otherCharacter->getBaseDefense()+m_constitution);
        otherCharacter->removeFromInventory(this);
        game->requestDeletion(this);
    }
}


bool A4ConstitutionPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"constitution")==0) {
        m_constitution = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4ConstitutionPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"constitution",m_constitution);
}


// ----------------------------------------------------------------


A4LifePotion::A4LifePotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
    m_life = v;
    m_gold = 50 + m_life*25;
}


void A4LifePotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        if (otherCharacter->getHp()==otherCharacter->getMaxHp()) {
            otherCharacter->setHp(otherCharacter->getHp()+m_life);
        }
        otherCharacter->setMaxHp(otherCharacter->getMaxHp()+m_life);
        otherCharacter->removeFromInventory(this);
        game->requestDeletion(this);
    }
}


bool A4LifePotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"life")==0) {
        m_life = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4LifePotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"life",m_life);
}


// ----------------------------------------------------------------


A4PowerPotion::A4PowerPotion(Symbol *name, Sort *sort, int v, Animation *a) : A4Potion(name,sort, a)
{
    m_power = v;
    m_gold = 75 + m_power*75;
}


void A4PowerPotion::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        if (otherCharacter->getMp()==otherCharacter->getMaxMp()) {
            otherCharacter->setMp(otherCharacter->getMp()+m_power);
        }
        otherCharacter->setMaxMp(otherCharacter->getMaxMp()+m_power);
        otherCharacter->removeFromInventory(this);
        game->requestDeletion(this);
    }
}


bool A4PowerPotion::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"power")==0) {
        m_power = atoi(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}


void A4PowerPotion::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"power",m_power);
}


