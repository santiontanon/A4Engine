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

#include "A4Spell.h"
#include "A4Character.h"

#include "A4AI.h"

A4Spell::A4Spell(Symbol *name, Sort *sort, int direction, int duration, int damage, float radius, A4Object *caster, Animation *a) : A4Object(name, sort)
{
    m_direction = direction;
    m_duration = duration*8;
    assert(damage>0);
    m_damage = damage;
    m_caster = caster;
    m_radius = radius;
    m_animations[A4_ANIMATION_MOVING] = a;
    m_currentAnimation = A4_ANIMATION_MOVING;

    m_walking_counter = 0;
    m_walk_speed = 8;
}


A4Spell::~A4Spell()
{
}


bool A4Spell::cycle(A4Game *game)
{
    int step = game->getTileDx() / getWalkSpeed();
    if (m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) step = game->getTileDy() / getWalkSpeed();
    m_x += A4Game::direction_x_inc[m_direction]*step;
    m_y += A4Game::direction_y_inc[m_direction]*step;
    if (m_map->spellCollision(this, m_caster)) {
        // check if we hit any character (radius)
        int rx = m_radius * game->getTileDx();
        int ry = m_radius * game->getTileDy();
        std::vector<A4Object *> *l = m_map->getAllObjects(m_x - rx, m_y - ry, getPixelWidth() + rx*2, getPixelHeight() + ry*2);
        for(A4Object *o:*l) {
            if (o->isCharacter() && (o!=m_caster || m_radius>0)) {
                A4Character *c = (A4Character *)o;
                // make sure the caster is still alive:
                if (!game->contains(m_caster)) m_caster = 0;
                assert(m_damage>0);
                if (c->takeDamage(m_damage)) { // spells ignore defense!

                    if (m_caster!=0 && m_caster->isCharacter()) {
                        ((A4Character *)m_caster)->gainXp(c->getGivesExperience());
                    }
                }
                if (m_caster!=0) {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, m_caster->getID(), m_caster->getSort()  , getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                    if (m_damage>0) {
                        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_attack, m_caster->getID(), m_caster->getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                    }
                }
            }
        }
        l->clear();
        delete l;
        return false;
    }
    m_duration--;
    if (m_duration<=0) return false;
    return true;
}


void A4Spell::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4Object::loadObjectAdditionalContent(xml,game, of, objectsToRevisit);
    
    // check if the spell has a caster:
    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        if (strcmp(a_name,"caster")==0) {
            XMLNode *copy = new XMLNode(xml);
            objectsToRevisit->push_back(std::pair<XMLNode *, A4Object *>(copy, this));
            break;
        }
    }
    delete attributes_xml;
}


bool A4Spell::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"direction")==0) {
        m_direction = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"duration")==0) {
        m_duration = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"damage")==0) {
        m_damage = atoi(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"radius")==0) {
        m_radius = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"caster")==0) {
        // this is loaded later
        return true;
    } else if (strcmp(a_name,"walking_counter")==0) {
        m_walking_counter = atof(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"walk_speed")==0) {
        m_walk_speed = atof(attribute_xml->get_attribute("value"));
        return true;
    }
    return false;
}

void A4Spell::revisitObject(XMLNode *xml, A4Game *game)
{
    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        if (strcmp(a_name,"caster")==0) {
            int o_ID = atoi(attribute_xml->get_attribute("value"));
            A4Object *o = game->getObject(o_ID);
            if (o==0) {
                output_debug_message("Revisiting A4Spell, and cannot find object with ID %i\n",o_ID);
            } else {
                m_caster = o;
            }
            break;
        }
    }
}


void A4Spell::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);
    
    saveObjectAttributeToXML(w,"direction",m_direction);
    saveObjectAttributeToXML(w,"duration",m_duration);
    saveObjectAttributeToXML(w,"damage",m_damage);
    saveObjectAttributeToXML(w,"radius",m_radius);
    if (m_caster!=0) saveObjectAttributeToXML(w,"caster",m_caster->getID());
    saveObjectAttributeToXML(w,"walking_counter",m_walking_counter);
    saveObjectAttributeToXML(w,"walk_speed",m_walk_speed);
}


