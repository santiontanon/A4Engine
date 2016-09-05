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
#include "A4Vehicle.h"

#include "A4ObjectFactory.h"

A4Vehicle::A4Vehicle(Symbol *name, Sort *sort) : A4WalkingObject(name, sort)
{
    m_currentAnimation = A4_ANIMATION_IDLE_RIGHT;

    m_hp = 10;
    m_magicImmune = false;

    m_walking_counter = 0;
}


A4Vehicle::~A4Vehicle()
{
// do not delete, since the objects loaded are still in the map
//    for(A4Object *o:m_load) delete o;
    m_load.clear();

}


bool A4Vehicle::takeDamage(int damage) {
    m_map->addDamageVisualization(new DamageVisualization(damage, m_x + getPixelWidth()/2, m_y, 50));
    m_hp -= damage;
    if (m_hp<=0) return true;
    return false;
}


void A4Vehicle::issueCommand(int command, int argument, int direction, A4Object *target, A4Game *game)
{
    if (m_state==A4CHARACTER_STATE_IN_VEHICLE) {
        if (command!=A4CHARACTER_COMMAND_TAKE) return;
    } else {
        if (m_state!=A4CHARACTER_STATE_IDLE) return;
    }
    switch(command) {
        case A4CHARACTER_COMMAND_WALK:
        case A4CHARACTER_COMMAND_ATTACK:
        case A4CHARACTER_COMMAND_INTERACT:
            m_direction_command_received_this_cycle[direction] = true;
            m_continuous_direction_command_max_movement[direction] = argument;
            break;
    }
}


bool A4Vehicle::cycle(A4Game *game)
{
    int movement_slack = getPixelWidth()*A4_MOVEMENT_SLACK;
    int v_movement_slack = getPixelHeight()*A4_MOVEMENT_SLACK;
    if (v_movement_slack>movement_slack) movement_slack = v_movement_slack;
    int max_movement_pixels_requested = 0;
    
    if (m_hp<=0) m_state = A4CHARACTER_STATE_DYING;

    // direction control:
    for(int i = 0;i<A4_NDIRECTIONS;i++) {
        if (m_direction_command_received_this_cycle[i]) {
            m_continuous_direction_command_timers[i]++;
        } else {
            m_continuous_direction_command_timers[i] = 0;
        }
    }
    if (m_state == A4CHARACTER_STATE_IDLE) {
        int most_recent_viable_walk_command = A4_DIRECTION_NONE;
        int timer = 0;
        for(int i = 0;i<A4_NDIRECTIONS;i++) {
            if (m_direction_command_received_this_cycle[i] && canMove(i, movement_slack)!=INT_MAX) {
                if (most_recent_viable_walk_command==A4_DIRECTION_NONE ||
                    m_continuous_direction_command_timers[i]<timer) {
                    most_recent_viable_walk_command = i;
                    timer = m_continuous_direction_command_timers[i];
                }
            }
        }
        if (most_recent_viable_walk_command!=A4_DIRECTION_NONE) {
            m_state = A4CHARACTER_STATE_WALKING;
            m_direction = most_recent_viable_walk_command;
            max_movement_pixels_requested = m_continuous_direction_command_max_movement[most_recent_viable_walk_command];
        }
    }
    for(int i = 0;i<A4_NDIRECTIONS;i++) m_direction_command_received_this_cycle[i] = false;

    if (m_state!=m_previous_state || m_direction!=m_previous_direction) m_state_cycle = 0;
    m_previous_state = m_state;
    m_previous_direction = m_direction;
    switch(m_state) {
        case A4CHARACTER_STATE_IDLE:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            break;
        case A4CHARACTER_STATE_WALKING:
        {
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_MOVING_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_MOVING_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_MOVING]!=0) {
                    m_currentAnimation = A4_ANIMATION_MOVING;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
                //                    m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;

            // the following kind of messy code just makes characters walk at the proper speed
            // it follows the idea of Brsenham's algorithms for proportionally scaling the speed of
            // the characters without using any floating point calculations.
            // it also makes the character move sideways a bit, if they need to align to fit through a corridor
            int step = game->getTileDx();
            if (m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) step = game->getTileDy();
            A4MapBridge *bridge = 0;
            int pixelsMoved = 0;
            int old_x = m_x;
            int old_y = m_y;
            while(m_walking_counter<=step) {
                int dir = m_direction;
                if (!WALK_TILE_BY_TILE) {
                    int tmp = canMove(m_direction, movement_slack);
                    if (tmp==INT_MAX) {
                        // we cannot move!
                        break;
                    } else {
                        if (tmp!=0) {
                            if ((m_direction==A4_DIRECTION_LEFT || m_direction==A4_DIRECTION_RIGHT) && tmp<0) dir = A4_DIRECTION_UP;
                            if ((m_direction==A4_DIRECTION_LEFT || m_direction==A4_DIRECTION_RIGHT) && tmp>0) dir = A4_DIRECTION_DOWN;
                            if ((m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) && tmp<0) dir = A4_DIRECTION_LEFT;
                            if ((m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) && tmp>0) dir = A4_DIRECTION_RIGHT;
                        }
                    }
                }
                m_x += A4Game::direction_x_inc[dir];
                m_y += A4Game::direction_y_inc[dir];
                m_walking_counter += getWalkSpeed();
                pixelsMoved++;
                if (!WALK_TILE_BY_TILE ||
                    ((m_x%game->getTileDx())==0 && (m_y%game->getTileDy())==0)) {
                    m_state = A4CHARACTER_STATE_IDLE;
                    if (WALK_TILE_BY_TILE) m_walking_counter = 0;
                    bridge = m_map->getBridge(m_x+getPixelWidth()/2,m_y+getPixelHeight()/2);
                    if (bridge!=0) {
                        // if we enter a bridge, but it's not with the first pixel we moved, then stop and do not go through the bridfge,
                        // to give the AI a chance to decide whether to go through the bridge or not
                        if (pixelsMoved>1) {
                            m_x -= A4Game::direction_x_inc[dir];
                            m_y -= A4Game::direction_y_inc[dir];
                            bridge = 0;
                        }
                        break;
                    }
                }
                
                // walk in blocks of a tile wide:
                if (A4Game::direction_x_inc[dir]!=0 && (m_x%game->getTileDx())==0) {
                    m_walking_counter = 0;
                    break;
                }
                if (A4Game::direction_y_inc[dir]!=0 && (m_y%game->getTileDy())==0) {
                    m_walking_counter = 0;
                    break;
                }
                if (max_movement_pixels_requested>0) {
                    max_movement_pixels_requested--;
                    if (max_movement_pixels_requested<=0) break;
                }
                //                    if (WALK_TILE_BY_TILE) if ((m_x%game->getTileDx())==0 && (m_y%game->getTileDy())==0) break;
            }
            if (m_walking_counter>=step) m_walking_counter-=step;
            if (bridge!=0) {
                //                        output_debug_message("Stepped over a bridge!\n");
                // teleport!
                int targetx,targety;
                if (bridge->m_linkedTo->findAvailableTargetLocation(this, game->getTileDx(), game->getTileDy(), targetx, targety)) {
                    game->requestWarp(this,bridge->m_linkedTo->getMap(), targetx, targety, m_layer);
                } else {
                    m_x = old_x;
                    m_y = old_y;
                    if (std::find(m_load.begin(), m_load.end(),game->getCurrentPlayer())!=m_load.end())
                        game->addMessage("Something is blocking the way!");
                }
            }
            break;
        }
        case A4CHARACTER_STATE_DYING:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_DEATH_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_DEATH_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_DEATH]!=0) {
                    m_currentAnimation = A4_ANIMATION_DEATH;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            if (m_state_cycle>=getWalkSpeed()) {
                // drop the load of characters:
                std::vector<A4Object *> l;    // we need "l" to avoic concurrent modifications of m_load
                for(A4Object *o:m_load) {
                    l.push_back(o);
                    game->requestWarp(o, m_map, m_x, m_y, o->getLayer());
                }
                m_load.clear();
                for(A4Object *o:l) {
                    ((A4Character *)o)->disembark();
                }
                l.clear();
                return false;
            }
            break;
    }

    return true;
}


void A4Vehicle::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4WalkingObject::loadObjectAdditionalContent(xml,game, of, objectsToRevisit);
    
    std::vector<XMLNode *> *l = xml->get_children("load");
    if (!l->empty()) {
        // add to revisit list:
        XMLNode *copy = new XMLNode(xml);
        objectsToRevisit->push_back(std::pair<XMLNode *, A4Object *>(copy, this));
    }
    delete l;
}


bool A4Vehicle::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4WalkingObject::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"hp")==0) {
        setHp(atoi(attribute_xml->get_attribute("value")));
        setMaxHp(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"max_hp")==0) {
        setMaxHp(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"magicImmune")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            setMagicImmune(true);
        } else {
            setMagicImmune(false);
        }
        return true;
    }
    return false;
}


void A4Vehicle::revisitObject(XMLNode *xml, A4Game *game)
{
    std::vector<XMLNode *> *l = xml->get_children("load");
    for(XMLNode *n:*l) {
        int o_ID = atoi(n->get_attribute("ID"));
        A4Object *o = game->getObject(o_ID);
        if (o==0) {
            output_debug_message("Revisiting A4Vehicle, and cannot find object with ID %i\n",o_ID);
        } else {
            m_load.push_back(o);
        }
    }
    delete l;
}


void A4Vehicle::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4WalkingObject::savePropertiesToXML(w,game);
    
    saveObjectAttributeToXML(w,"hp",m_hp);
    saveObjectAttributeToXML(w,"max_hp",m_max_hp);
    
    saveObjectAttributeToXML(w,"magicImmune",m_magicImmune);
    
    for(A4Object *o:m_load) {
        w->openTag("load");
        w->setAttribute("ID", o->getID());
        w->closeTag("load");
    }
}


