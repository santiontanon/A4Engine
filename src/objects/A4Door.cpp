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

#include "A4Door.h"
#include "A4Item.h"
#include "A4Key.h"
#include "A4Character.h"
#include "A4Vehicle.h"

A4Door::A4Door(Sort *sort, Symbol *ID, bool closed, bool consumeKey, class Animation *a_closed, Animation *a_open) : A4Object(0, sort)
{
    m_name = new Symbol("door");
	m_doorID = ID;
	m_closed = closed;
    m_consumeKey = consumeKey;
	m_animations[A4_ANIMATION_CLOSED] = a_closed;
	m_animations[A4_ANIMATION_OPEN] = a_open;
	if (m_closed) m_currentAnimation = A4_ANIMATION_CLOSED;
		 	 else m_currentAnimation = A4_ANIMATION_OPEN;
    m_doorGroupID = 0;
}


A4Door::~A4Door()
{
	delete m_doorID;
    if (m_doorGroupID!=0) delete m_doorGroupID;
}


void A4Door::event(int a_event, class A4Character *character, class A4Map *map, class A4Game *game)
{
	A4Object::event(a_event,character,map,game);

	if (a_event==A4_EVENT_INTERACT) {
        if (m_consumeKey && !m_closed) return;  // if it consumes the key, it cannot be reopened!
		// see if the character has the key:
		for(A4Object *o:*(character->getInventory())) {
			if (o->isKey()) {
				A4Key *key = (A4Key *)o;
				if (key->getID()->cmp(m_doorID)) {
					// the player has the proper key!
                    if (checkForBlockages(!m_closed, character, map, game)) {
                        // change all the doors in the same doorgroup:
                        if (m_doorGroupID==0) {
                            changeStateRecursively(!m_closed, character, map, game);
                            if (m_consumeKey) {
                                character->removeFromInventory(key);
                                game->requestDeletion(key);
                            }
                        } else {
                            if (game->checkIfDoorGroupStateCanBeChanged(m_doorGroupID, m_closed, character)) {
                                changeStateRecursively(!m_closed, character, map, game);
                                game->setDoorGroupState(m_doorGroupID, m_closed, character);
                                if (m_consumeKey) {
                                    character->removeFromInventory(key);
                                    game->requestDeletion(key);
                                }
                            }
                        }
                    }
					break;
				}
			}
		}
	}
}


void A4Door::changeStateRecursively(bool closed, class A4Character *character, class A4Map *map, class A4Game *game)
{
    if (m_closed==closed) return;

    eventWithID(A4_EVENT_OPEN, m_doorID->get(), character, map, game);

    int dx1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelWidth());
    int dy1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelHeight());
    int dx2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelWidth());
    int dy2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelHeight());

    int dx = (dx1>dx2 ? dx1:dx2);
    int dy = (dy1>dy2 ? dy1:dy2);

    for(int i = 0;i<A4_NDIRECTIONS;i++) {
        std::vector<A4Object *> *l = m_map->getAllObjects(m_x+A4Game::direction_x_inc[i], m_y+A4Game::direction_y_inc[i], dx, dy);

        for(A4Object *o:*l) {
            if (o!=this && o->isDoor()) {
                A4Door *door = (A4Door *)o;
                if (door->m_doorID->cmp(m_doorID)) door->changeStateRecursively(closed, character, map, game);
            }
        }

        l->clear();
        delete l;
    }
}


bool A4Door::checkForBlockages(bool closed, A4Character *character, A4Map *map, A4Game *game)
{
    std::list<A4Door *> l;
    
    return checkForBlockages(closed, character, map, game, &l);
}


bool A4Door::checkForBlockages(bool closed, A4Character *character, A4Map *map, A4Game *game, std::list<A4Door *> *alreadyVisited)
{
    if (closed) {
        for(A4Door *d:*alreadyVisited) {
            if (this==d) return true;
        }
        alreadyVisited->push_back(this);
        
        // closing the doors:
        bool blockage = false;
        std::vector<A4Object *> *l = m_map->getAllObjectCollisions(this);
        for(A4Object *caught:*l) {
            if (caught->isCharacter()) {
                blockage = true;
            } else if (caught->isVehicle()) {
                blockage = true;
            }
        }
        l->clear();
        delete l;
        
        int dx1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelWidth());
        int dy1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelHeight());
        int dx2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelWidth());
        int dy2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelHeight());
        int dx = (dx1>dx2 ? dx1:dx2);
        int dy = (dy1>dy2 ? dy1:dy2);
        for(int i = 0;i<A4_NDIRECTIONS;i++) {
            std::vector<A4Object *> *l = m_map->getAllObjects(m_x+A4Game::direction_x_inc[i], m_y+A4Game::direction_y_inc[i], dx, dy);
            
            for(A4Object *o:*l) {
                if (o!=this && o->isDoor()) {
                    A4Door *door = (A4Door *)o;
                    if (door->m_doorID->cmp(m_doorID)) {
                        if (!door->checkForBlockages(closed, character, map, game, alreadyVisited)) {
                            blockage = true;
                        }
                    }
                }
            }
            
            l->clear();
            delete l;
        }
        
        return !blockage;
    } else {
        // opening the doors:
        return true;
    }
}


void A4Door::eventWithID(int a_event, char *ID, class A4Character *character, class A4Map *map, class A4Game *game)
{
	A4Object::eventWithID(a_event,ID,character,map,game);

	if (a_event == A4_EVENT_OPEN && m_doorID->cmp(ID)) {
		for(A4EventRule *rule:m_event_scripts[a_event]) {
			rule->executeEffects(this, map, game, character);
		}

		m_closed = (m_closed ? false:true);
		if (m_closed) {
			m_currentAnimation = A4_ANIMATION_CLOSED;
			event(A4_EVENT_CLOSE,character,map,game);
		} else {
		 	m_currentAnimation = A4_ANIMATION_OPEN;
			event(A4_EVENT_OPEN,character,map,game);
		}

		if (character!=0) {
			map->reevaluateVisibility();
		}
	}
}


int A4Door::getPixelWidth()
{
    if (m_pixel_width_cache_cycle == m_cycle) return m_pixel_width_cache;
    int dx1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelWidth());
    int dy1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelHeight());
    int dx2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelWidth());
    int dy2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelHeight());
    int dx = (dx1>dx2 ? dx1:dx2);
    int dy = (dy1>dy2 ? dy1:dy2);
    m_pixel_width_cache = dx;
    m_pixel_height_cache = dy;
    m_pixel_width_cache_cycle = m_cycle;
    return m_pixel_width_cache;
}


int A4Door::getPixelHeight()
{
    if (m_pixel_width_cache_cycle == m_cycle) return m_pixel_width_cache;
    int dx1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelWidth());
    int dy1 = (m_animations[A4_ANIMATION_CLOSED]==0 ? 0:m_animations[A4_ANIMATION_CLOSED]->getPixelHeight());
    int dx2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelWidth());
    int dy2 = (m_animations[A4_ANIMATION_OPEN]==0 ? 0:m_animations[A4_ANIMATION_OPEN]->getPixelHeight());
    int dx = (dx1>dx2 ? dx1:dx2);
    int dy = (dy1>dy2 ? dy1:dy2);
    m_pixel_width_cache = dx;
    m_pixel_height_cache = dy;
    m_pixel_width_cache_cycle = m_cycle;
    return m_pixel_height_cache;
}


bool A4Door::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"doorID")==0) {
        if (m_doorID!=0) delete m_doorID;
        m_doorID = new Symbol(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"doorgroup")==0) {
        if (m_doorGroupID!=0) delete m_doorGroupID;
        m_doorGroupID = new Symbol(attribute_xml->get_attribute("value"));
        return true;
    } else if (strcmp(a_name,"closed")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            m_closed = true;
        } else {
            m_closed = false;
        }
        if (m_closed) m_currentAnimation = A4_ANIMATION_CLOSED;
                 else m_currentAnimation = A4_ANIMATION_OPEN;
        return true;
    } else if (strcmp(a_name,"consumeKey")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            m_consumeKey = true;
        } else {
            m_consumeKey = false;
        }
        return true;
    }
    return false;
}


void A4Door::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w, game);

    saveObjectAttributeToXML(w,"doorID",m_doorID->get());
    if (m_doorGroupID!=0) saveObjectAttributeToXML(w,"doorgroup",m_doorGroupID->get());
    saveObjectAttributeToXML(w,"closed",m_closed);
    saveObjectAttributeToXML(w,"consumeKey",m_consumeKey);
}
