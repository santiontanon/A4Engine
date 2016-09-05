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
#include "string.h"

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
#include "A4Object.h"
#include "A4Game.h"
#include "Ontology.h"


int A4Object::s_nextID = 1;

A4Object::A4Object(Symbol *name, Sort *sort)
{
    m_ID = s_nextID++;
	if (name!=0) m_name = new Symbol(name);
			else m_name = 0;
    m_sort = sort;
	m_x = m_y = 0;
	m_map = 0;
	m_animations = new Animation *[A4_N_ANIMATIONS];
	for(int i = 0;i<A4_N_ANIMATIONS;i++) m_animations[i] = 0;
	m_currentAnimation = A4_ANIMATION_IDLE;
    m_cycle = 0;

	m_gold = 0;
	m_takeable = false;
	m_usable = false;
    m_deadRequest = false;
    m_burrowed = false;

    m_canWalk = true;
    m_canSwim = false;
    
    m_pixel_width_cache_cycle = -1;
}


A4Object::~A4Object()
{
//    output_debug_message("Deleting A4Object %p (%s)\n",this,(m_name==0 ? "-":m_name->get()));
	if (m_name!=0) delete m_name;
	m_name = 0;
    m_sort = 0;
	for(int i = 0;i<A4_N_ANIMATIONS;i++) {
		if (m_animations[i]!=0) delete m_animations[i];
		m_animations[i] = 0;
	}
	if (m_animations!=0) delete m_animations;
	m_animations = 0;
	for(int i = 0;i<A4_NEVENTS;i++) {
		for(A4EventRule *s:m_event_scripts[i]) delete s;
		m_event_scripts[i].clear();
	}

    for(A4ScriptExecutionQueue *a:m_script_queues) delete a;
    m_script_queues.clear();
    for(char *a:m_storyStateVariables) delete a;
    m_storyStateVariables.clear();
    for(char *a:m_storyStateValues) delete a;
    m_storyStateValues.clear();
    
    for(Agenda *a:m_agendas) delete a;
    m_agendas.clear();
}


bool A4Object::cycle(A4Game *game)
{
    if (m_cycle==0) event(A4_EVENT_START, 0, m_map, game);

    for(A4EventRule *r:m_event_scripts[A4_EVENT_TIMER]) r->execute(this,m_map,game,0);
    for(A4EventRule *r:m_event_scripts[A4_EVENT_STORYSTATE]) r->execute(this,m_map,game,0);
    std::vector<Agenda *> toDelete;
    for(Agenda *a:m_agendas) {
        if (a->execute(this,m_map,game,0)) toDelete.push_back(a);
    }
    for(Agenda *a:toDelete) m_agendas.remove(a);
    
    executeScriptQueues(game);

    if (m_animations[m_currentAnimation]!=0) m_animations[m_currentAnimation]->cycle();

    if (m_deadRequest) return false;

    m_cycle++;
	return true;
}


void A4Object::executeScriptQueues(A4Game *game) {
    std::vector<A4ScriptExecutionQueue *> toDelete;
    for(A4ScriptExecutionQueue *seb:m_script_queues) {
        while(true) {
            A4Script *s = seb->scripts.front();
            int retval = s->execute((seb->object == 0 ? this:seb->object),
                                    (seb->map == 0 ? m_map:seb->map),
                                    (seb->game == 0 ? game:seb->game),
                                    seb->otherCharacter);
            if (retval==SCRIPT_FINISHED) {
                seb->scripts.pop_front();
                delete s;
                if (seb->scripts.empty()) {
                    toDelete.push_back(seb);
                    break;
                }
            } else if (retval==SCRIPT_NOT_FINISHED) {
                break;
            } else if (retval==SCRIPT_FAILED) {
                toDelete.push_back(seb);
                break;
            }
        }
    }
    for(A4ScriptExecutionQueue *seb:toDelete) {
        m_script_queues.remove(seb);
        delete seb;
    }
    toDelete.clear();
}


void A4Object::draw(int offsetx, int offsety, float zoom, class A4Game *game)
{
	if (m_currentAnimation>=0 && m_animations[m_currentAnimation]!=0) {
		m_animations[m_currentAnimation]->draw((m_x + offsetx)*zoom, (m_y + offsety)*zoom, zoom);
	}
}


void A4Object::event(int event, A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
	for(A4EventRule *rule:m_event_scripts[event]) {
		rule->executeEffects(this, map, game, otherCharacter);
	}
}

void A4Object::eventWithID(int event, char *ID, A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{

}


void A4Object::eventWithObject(int event, A4Character *otherCharacter, A4Object *object, class A4Map *map, class A4Game *game)
{
    for(A4EventRule *rule:m_event_scripts[event]) {
        if (event==A4_EVENT_RECEIVE ||
            event==A4_EVENT_ACTION_TAKE ||
            event==A4_EVENT_ACTION_DROP ||
            event==A4_EVENT_ACTION_USE ||
            event==A4_EVENT_ACTION_EQUIP ||
            event==A4_EVENT_ACTION_UNEQUIP ||
            event==A4_EVENT_ACTION_INTERACT ||
            event==A4_EVENT_ACTION_CHOP ||
            event==A4_EVENT_ACTION_GIVE ||
            event==A4_EVENT_ACTION_SELL ||
            event==A4_EVENT_ACTION_BUY) {
            if (rule->getItem()==0 ||
                strcmp(object->getName()->get(),rule->getItem())==0 ||
                object->is_a(rule->getItem())) {
                rule->executeEffects(this, map, game, otherCharacter);
            }
        } else {
            output_debug_message("eventWithObject for event %i, is undefined\n", event);
            exit(1);
        }
    }
}


void A4Object::eventWithInteger(int event, int value, A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
    for(A4EventRule *rule:m_event_scripts[event]) {
        if (event==A4_EVENT_ACTION_SPELL) {
            if (rule->getSpell()==-1 || rule->getSpell() == value) {
                rule->executeEffects(this, map, game, otherCharacter);
            }
        } else if (event==A4_EVENT_ACTION_ATTACK) {
            if (rule->getHit()==-1 || rule->getHit() == value) {
                rule->executeEffects(this, map, game, otherCharacter);
            }
        } else {
            output_debug_message("eventWithInteger for event %i, is undefined\n", event);
            exit(1);
        }
    }
}


void A4Object::eventWithSpeechAct(int event, SpeechAct *speechAct, bool angry, class A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
    for(A4EventRule *rule:m_event_scripts[event]) {
        if (event==A4_EVENT_ACTION_TALK) {
            if (rule->getCharacter()==0 ||
                (otherCharacter!=0 &&
                 (strcmp(((A4Object *)otherCharacter)->getName()->get(),rule->getCharacter())==0 ||
                  ((A4Object *)otherCharacter)->is_a(rule->getCharacter())))) {
                if (rule->getAngry()==-1 ||
                    (rule->getAngry()==0 && !angry) ||
                    (rule->getAngry()==1 && angry)) {
                    if (rule->getPerformative()==-1 ||
                        speechAct->performative == rule->getPerformative()) {
                        rule->executeEffects(this, map, game, otherCharacter);
                    }
                }
            }
        } else {
            output_debug_message("eventWithSpeechAct for event %i, is undefined\n", event);
            exit(1);
        }
    }
}




void A4Object::setStoryStateVariable(const char *variable, const char *value)
{
    std::vector<char *>::iterator pos1 = m_storyStateVariables.begin();
    std::vector<char *>::iterator pos2 = m_storyStateValues.begin();

    while(!(pos1==m_storyStateVariables.end()) && strcmp(*pos1,variable)!=0) {
        pos1++;
        pos2++;
    }

    if (pos1==m_storyStateVariables.end()) {
        // store a new value:
        char *tmp1 = new char[strlen(variable)+1];
        strcpy(tmp1, variable);
        char *tmp2 = new char[strlen(value)+1];
        strcpy(tmp2, value);
        m_storyStateVariables.push_back(tmp1);
        m_storyStateValues.push_back(tmp2);
    } else {
        char *tmp2 = new char[strlen(value)+1];
        strcpy(tmp2, value);
        *pos2 = tmp2;
    }
}


char *A4Object::getStoryStateVariable(const char *variable)
{
    std::vector<char *>::iterator pos1 = m_storyStateVariables.begin();
    std::vector<char *>::iterator pos2 = m_storyStateValues.begin();

    while(!(pos1==m_storyStateVariables.end()) && strcmp(*pos1,variable)!=0) {
        pos1++;
        pos2++;
    }

    if (pos1==m_storyStateVariables.end()) {
        return 0;
    } else {
        return *pos2;
    }
}



void A4Object::clearOpenGLState()
{
}


void A4Object::warp(int x, int y, A4Map *map, int layer)
{
	if (m_map!=0) m_map->removeObject(this);
	m_x = x;
	m_y = y;
	m_map = map;
	if (m_map!=0) m_map->addObject(this, layer);
}


void A4Object::setAnimation(int idx, Animation *a)
{
    if (m_animations[idx]!=0) {
        output_debug_message("Overwriting animation %i of object %s!\n",idx, m_name->get());
        delete m_animations[idx];
    }
	m_animations[idx] = a;
}


int A4Object::getPixelWidth()
{
    if (m_pixel_width_cache_cycle == m_cycle) return m_pixel_width_cache;
	if (m_currentAnimation<0) return 0;
	Animation *a = m_animations[m_currentAnimation];
	if (a==0) return 0;
    m_pixel_width_cache = a->getPixelWidth();
    m_pixel_height_cache = a->getPixelHeight();
    m_pixel_width_cache_cycle = m_cycle;
    return m_pixel_width_cache;;
}


int A4Object::getPixelHeight()
{
    if (m_pixel_width_cache_cycle == m_cycle) return m_pixel_height_cache;
	if (m_currentAnimation<0) return 0;
	Animation *a = m_animations[m_currentAnimation];
	if (a==0) return 0;
    m_pixel_width_cache = a->getPixelWidth();
    m_pixel_height_cache = a->getPixelHeight();
    m_pixel_width_cache_cycle = m_cycle;
    return m_pixel_height_cache;;
}


bool A4Object::collision(int x2, int y2, int dx2, int dy2)
{
	int dx = getPixelWidth();
	int dy = getPixelHeight();
	if (m_x+dx>x2 && x2+dx2>m_x &&
		m_y+dy>y2 && y2+dy2>m_y) return true;
	return false;
}


bool A4Object::collision(A4Object *o2)
{
	int dx = getPixelWidth();
	int dy = getPixelHeight();
	int dx2 = o2->getPixelWidth();
	int dy2 = o2->getPixelHeight();
	if (m_x+dx>o2->m_x && o2->m_x+dx2>m_x &&
		m_y+dy>o2->m_y && o2->m_y+dy2>m_y) return true;
	return false;
}


bool A4Object::collision(int xoffs, int yoffs, A4Object *o2)
{
    int dx = getPixelWidth();
    int dy = getPixelHeight();
    int dx2 = o2->getPixelWidth();
    int dy2 = o2->getPixelHeight();
    if (m_x+xoffs+dx>o2->m_x && o2->m_x+dx2>m_x+xoffs &&
        m_y+yoffs+dy>o2->m_y && o2->m_y+dy2>m_y+yoffs) return true;
    return false;
}


int A4Object::pixelDistance(A4Object *o2)
{
    int dx = 0;
    if (getX() > o2->getX()+o2->getPixelWidth()) {
        dx = getX() - (o2->getX()+o2->getPixelWidth());
    } else if (o2->getX() > getX()+getPixelWidth()) {
        dx = o2->getX() - (getX()+getPixelWidth());
    }
    int dy = 0;
    if (getY() > o2->getY()+o2->getPixelHeight()) {
        dy = getY() - (o2->getY()+o2->getPixelHeight());
    } else if (o2->getY() > getY()+getPixelHeight()) {
        dy = o2->getY() - (getY()+getPixelHeight());
    }
    return dx+dy;
}


int A4Object::canMove(int direction, int slack)
{
    return canMove(direction,slack,false);
}


int A4Object::canMove(int direction, int slack, bool treatBridgesAsWalls)
{
    if (treatBridgesAsWalls) {
        if (slack==0) {
            if (canMoveWithoutGoingThroughABridge(direction)) return 0;
            return INT_MAX;
        } else {
            output_debug_message("A4Object::canMove with slack!=0 and treatBridgesAsWalls==true not supported!");
            exit(1);
        }
    }
    
    if (WALK_TILE_BY_TILE) {
        if (m_map->walkable(m_x+A4Game::direction_x_inc[direction]*getPixelWidth(),
                            m_y+A4Game::direction_y_inc[direction]*getPixelHeight(),
                            getPixelWidth(),
                            getPixelHeight(),this)) return 0;
    } else {
        if (direction==A4_DIRECTION_LEFT) {
            bool positiveStillGood = true;
            bool negativeStillGood = true;
            for(int i = 0;i<=slack;i++) {
                if (positiveStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x,m_y+i,getPixelWidth(),getPixelHeight(),this))
                    positiveStillGood = false;
                if (negativeStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x,m_y-i,getPixelWidth(),getPixelHeight(),this))
                    negativeStillGood = false;
                if (positiveStillGood &&        m_map->walkableConsideringVehicles(m_x-1,m_y+i,getPixelWidth(),getPixelHeight(),this)) return i;
                if (negativeStillGood && i>0 && m_map->walkableConsideringVehicles(m_x-1,m_y-i,getPixelWidth(),getPixelHeight(),this)) return -i;
            }
        } else if (direction==A4_DIRECTION_UP) {
            bool positiveStillGood = true;
            bool negativeStillGood = true;
            for(int i = 0;i<=slack;i++) {
                if (positiveStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x+i,m_y,getPixelWidth(),getPixelHeight(),this))
                    positiveStillGood = false;
                if (negativeStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x-i,m_y,getPixelWidth(),getPixelHeight(),this))
                    negativeStillGood = false;
                if (positiveStillGood &&        m_map->walkableConsideringVehicles(m_x+i,m_y-1,getPixelWidth(),getPixelHeight(),this)) return i;
                if (negativeStillGood && i>0 && m_map->walkableConsideringVehicles(m_x-i,m_y-1,getPixelWidth(),getPixelHeight(),this)) return -i;
            }
        } else if (direction==A4_DIRECTION_RIGHT) {
            bool positiveStillGood = true;
            bool negativeStillGood = true;
            for(int i = 0;i<=slack;i++) {
                if (positiveStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x,m_y+i,getPixelWidth(),getPixelHeight(),this))
                    positiveStillGood = false;
                if (negativeStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x,m_y-i,getPixelWidth(),getPixelHeight(),this))
                    negativeStillGood = false;
                if (positiveStillGood &&        m_map->walkableConsideringVehicles(m_x+1,m_y+i,getPixelWidth(),getPixelHeight(),this)) return i;
                if (negativeStillGood && i>0 && m_map->walkableConsideringVehicles(m_x+1,m_y-i,getPixelWidth(),getPixelHeight(),this)) return -i;
            }
        } else if (direction==A4_DIRECTION_DOWN) {
            bool positiveStillGood = true;
            bool negativeStillGood = true;
            for(int i = 0;i<=slack;i++) {
                if (positiveStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x+i,m_y,getPixelWidth(),getPixelHeight(),this))
                    positiveStillGood = false;
                if (negativeStillGood && i>0 && !m_map->walkableConsideringVehicles(m_x-i,m_y,getPixelWidth(),getPixelHeight(),this))
                    negativeStillGood = false;
                if (positiveStillGood &&        m_map->walkableConsideringVehicles(m_x+i,m_y+1,getPixelWidth(),getPixelHeight(),this)) return i;
                if (negativeStillGood && i>0 && m_map->walkableConsideringVehicles(m_x-i,m_y+1,getPixelWidth(),getPixelHeight(),this)) return -i;
            }
        }
    }
    return INT_MAX;
}


bool A4Object::canMoveWithoutGoingThroughABridge(int direction)
{
    if (WALK_TILE_BY_TILE) {
        if (m_map->getBridge(m_x+A4Game::direction_x_inc[direction]*getPixelWidth() + getPixelWidth()/2,
                             m_y+A4Game::direction_y_inc[direction]*getPixelHeight() + getPixelHeight()/2)!=0) return false;
        if (m_map->walkableConsideringVehicles(m_x+A4Game::direction_x_inc[direction]*getPixelWidth(),
                                               m_y+A4Game::direction_y_inc[direction]*getPixelHeight(),
                                               getPixelWidth(),
                                               getPixelHeight(),this)) return true;
    } else {
        if (direction==A4_DIRECTION_LEFT) {
            if (m_map->getBridge(m_x-1 + getPixelWidth()/2,m_y+getPixelHeight()/2)!=0) return false;
            if (m_map->walkableConsideringVehicles(m_x-1,m_y,getPixelWidth(),getPixelHeight(),this)) return true;
            return false;
        } else if (direction==A4_DIRECTION_UP) {
            if (m_map->getBridge(m_x + getPixelWidth()/2,m_y-1 + getPixelHeight()/2)!=0) return false;
            if (m_map->walkableConsideringVehicles(m_x,m_y-1,getPixelWidth(),getPixelHeight(),this)) return true;
            return false;
        } else if (direction==A4_DIRECTION_RIGHT) {
            if (m_map->getBridge(m_x+1 + getPixelWidth()/2,m_y+getPixelHeight()/2)!=0) return false;
            if (m_map->walkableConsideringVehicles(m_x+1,m_y,getPixelWidth(),getPixelHeight(),this)) return true;
            return false;
        } else if (direction==A4_DIRECTION_DOWN) {
            if (m_map->getBridge(m_x + getPixelWidth()/2,m_y+1 + getPixelHeight()/2)!=0) return false;
            if (m_map->walkableConsideringVehicles(m_x,m_y+1,getPixelWidth(),getPixelHeight(),this)) return true;
            return false;
        }
    }
    return false;
}



bool A4Object::seeThrough()
{
	Animation *a = m_animations[m_currentAnimation];
	if (a==0) return true;

	// this assumes that if the object has size > 1, all the tile have the same "seeThrough"
	int tile = a->getTile();
	if (tile<0) return true;

	GraphicFile *gf = a->getGraphicFile();
	if (gf->m_tileSeeThrough[tile]==1) return false;
	return true;
}


void A4Object::addAgenda(Agenda *a)
{
    for(Agenda *a2:m_agendas) {
        if (strcmp(a2->name,a->name)==0) {
            m_agendas.remove(a);
            break;
        }
    }
    m_agendas.push_back(a);
}


void A4Object::removeAgenda(char *agenda)
{
    for(Agenda *a:m_agendas) {
        if (strcmp(agenda,a->name)==0) {
            m_agendas.remove(a);
            return;
        }
    }
}


void A4Object::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    // add animations:
    std::vector<XMLNode *> *animations_xml = xml->get_children("animation");
    for(XMLNode *animation_xml:*animations_xml) {
        Animation *a = new Animation(animation_xml, game);
        
        for(int idx = 0;idx<A4_N_ANIMATIONS;idx++) {
            if (strcmp(A4Game::animationNames[idx],animation_xml->get_attribute("name"))==0) {
                setAnimation(idx, a);
                a = 0;
                break;
            }
        }
        if (a!=0) output_debug_message("A4ObjectFactory: not supported animation %s\n", animation_xml->get_attribute("name"));
    }
    delete animations_xml;
        
    // set attributes:
    bool canWalkSet = false;
    bool canSwimSet = false;
    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        
        if (strcmp(a_name,"canSwim")==0) {
            canSwimSet = true;
            if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
                setCanSwim(true);
            } else {
                setCanSwim(false);
            }
        } else if (strcmp(a_name,"canWalk")==0) {
            canWalkSet = true;
            if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
                setCanWalk(true);
            } else {
                setCanWalk(false);
            }
        } else if (!loadObjectAttribute(attribute_xml)) {
            output_debug_message("Unknown attribute: %s\n", a_name);
            exit(1);
        }
    }
    if (canWalkSet) {
        if (canSwimSet) {
            // good, do nothing
        } else {
            setCanSwim(!getCanWalk());
        }
    } else {
        if (canSwimSet) {
            setCanWalk(!getCanSwim());
        } else {
            // default:
//            setCanWalk(true);
//            setCanSwim(false);
        }
    }
    delete attributes_xml;
        
    // loading scripts:
    {
        // on start:
        std::vector<XMLNode *> *onstarts_xml = xml->get_children("onStart");
        for(XMLNode *onstart_xml:*onstarts_xml) {
            A4ScriptExecutionQueue *tmpq = 0;
            for(XMLNode *script_xml:*(onstart_xml->get_children())) {
                A4Script *s = new A4Script(script_xml);
                if (tmpq==0) tmpq = new A4ScriptExecutionQueue(this, 0, 0, 0);
                tmpq->scripts.push_back(s);
            }
            if (tmpq!=0) m_script_queues.push_back(tmpq);
        }
        delete onstarts_xml;
        
        // on end:
        std::vector<XMLNode *> *onends_xml = xml->get_children("onEnd");
        for(XMLNode *onend_xml:*onends_xml) {
            for(XMLNode *script_xml:*(onend_xml->get_children())) {
                A4Script *s = new A4Script(script_xml);
                m_event_scripts[A4_EVENT_END].push_back(new A4EventRule(A4_EVENT_END,s, false));
            }
        }
        delete onends_xml;
                
        // event rules:
        std::vector<XMLNode *> *eventrules_xml = xml->get_children("eventRule");
        for(XMLNode *rule_xml:*eventrules_xml) {
            A4EventRule *r = new A4EventRule(rule_xml);
            m_event_scripts[r->getEvent()].push_back(r);
        }
        delete eventrules_xml;        
    }
}


bool A4Object::loadObjectAttribute(XMLNode *attribute_xml)
{
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"ID")==0) {
        m_ID = atoi(attribute_xml->get_attribute("value"));
        if (A4Object::s_nextID<=m_ID) A4Object::s_nextID = m_ID+1;
        return true;
    } else if (strcmp(a_name,"name")==0) {
        setName(new Symbol(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"gold")==0 ||
               strcmp(a_name,"value")==0) {
        setGold(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"takeable")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            setTakeable(true);
        } else {
            setTakeable(false);
        }
        return true;
    } else if (strcmp(a_name,"usable")==0 ||
               strcmp(a_name,"useable")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            setUsable(true);
        } else {
            setUsable(false);
        }
        return true;
    } else if (strcmp(a_name,"burrowed")==0) {
        m_burrowed = false;
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) m_burrowed = true;
        return true;
    }
    return false;
}


void A4Object::revisitObject(XMLNode *xml, A4Game *game)
{
}


void A4Object::objectRemoved(A4Object *o)
{
}


void A4Object::saveToXML(XMLwriter *w, A4Game *game, int type, bool saveLocation)
{
//    Ontology *o = game->getOntology();
    
    w->openTag("object");
    if (type==0) {
        w->setAttribute("class", m_sort->getName()->get());
        w->setAttribute("completeRedefinition", "true");
    } else if (type==1) {
        w->setAttribute("name", m_name->get());
        w->setAttribute("class", "Bridge");
    } else if (type==2) {
        w->setAttribute("name", m_name->get());
        w->setAttribute("class", "BridgeDestination");
    }

    if (saveLocation) {
        w->setAttribute("x", getX());
        w->setAttribute("y", getY());
        w->setAttribute("width", getPixelWidth());
        w->setAttribute("height", getPixelHeight());
    }
    
    savePropertiesToXML(w,game);
    w->closeTag("object");
}


void A4Object::saveToXMLForMainFile(class XMLwriter *w, A4Game *game, int mapNumber)
{
    w->openTag("player");
    w->setAttribute("class", m_sort->getName()->get());
    w->setAttribute("completeRedefinition", "true");
    w->setAttribute("x", m_x);
    w->setAttribute("y", m_y);
    w->setAttribute("map", mapNumber);
    savePropertiesToXML(w,game);
    w->closeTag("player");
}


void A4Object::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    for(int i = 0;i<A4_N_ANIMATIONS;i++) {
        if (m_animations[i]!=0) {
            m_animations[i]->saveToXML(w, A4Game::animationNames[i]);
        }
    }

    saveObjectAttributeToXML(w,"ID",m_ID);
    if (m_name!=0) saveObjectAttributeToXML(w,"name",m_name->get());
    if (m_gold!=0) saveObjectAttributeToXML(w,"gold",m_gold);
    if (isCharacter() || isVehicle()) {
        saveObjectAttributeToXML(w,"canWalk",getCanWalk());
        saveObjectAttributeToXML(w,"canSwim",getCanSwim());
    }
    saveObjectAttributeToXML(w,"takeable",getTakeable());
    saveObjectAttributeToXML(w,"usable",getUsable());
    saveObjectAttributeToXML(w,"burrowed",getBurrowed());
    
    if (!m_storyStateVariables.empty()) {
        w->openTag("onStart");
        std::vector<char *>::iterator it1 = m_storyStateVariables.begin();
        std::vector<char *>::iterator it2 = m_storyStateValues.begin();
        while(it1!=m_storyStateVariables.end()) {
            w->openTag("storyState");
            w->setAttribute("variable", *it1);
            w->setAttribute("value", *it2);
            w->setAttribute("scope", "game");
            w->closeTag("storyState");
            it1++;
            it2++;
        }
        w->closeTag("onStart");
    }
    
    // each execution queue goes to its own "onStart" block:
    for(A4ScriptExecutionQueue *seq:m_script_queues) {
        w->openTag("onStart");
        for(A4Script *s:seq->scripts) s->saveToXML(w);
        w->closeTag("onStart");
    }
    if (m_deadRequest) {
        w->openTag("onStart");
        w->openTag("die");
        w->closeTag("die");
        w->closeTag("onStart");
    }
    
    if (!m_agendas.empty()) {
        w->openTag("onStart");
        // create a script for the agenda
        for(Agenda *a:m_agendas) {
            w->openTag("addAgenda");
            w->setAttribute("agenda", a->name);
            w->setAttribute("duration", a->duration);
            w->setAttribute("loop", a->loop ? "true":"false");
            w->setAttribute("cycle", a->cycle);
            for(AgendaEntry *ae:a->m_entries) {
                w->openTag("entry");
                w->setAttribute("time", ae->time);
                for(A4Script *s:ae->m_scripts) {
                    s->saveToXML(w);
                }
                w->closeTag("entry");
            }
            w->closeTag("addAgenda");
        }
        w->closeTag("onStart");
    }
    
    // rules:
    for(int i = 0;i<A4_NEVENTS;i++) {
        for(A4EventRule *er:m_event_scripts[i]) {
            er->saveToXML(w);
        }
    }
}

void A4Object::saveObjectAttributeToXML(class XMLwriter *w, const char *attribute, const char *value)
{
    w->openTag("attribute");
    w->setAttribute("name", attribute);
    w->setAttribute("value", value);
    w->closeTag("attribute");
}


void A4Object::saveObjectAttributeToXML(class XMLwriter *w, const char *attribute, bool value)
{
    w->openTag("attribute");
    w->setAttribute("name", attribute);
    w->setAttribute("value", value ? "true":"false");
    w->closeTag("attribute");
}


void A4Object::saveObjectAttributeToXML(class XMLwriter *w, const char *attribute, int value)
{
    w->openTag("attribute");
    w->setAttribute("name", attribute);
    w->setAttribute("value", value);
    w->closeTag("attribute");
}


void A4Object::saveObjectAttributeToXML(class XMLwriter *w, const char *attribute, float value)
{
    char tmp[64];
    w->openTag("attribute");
    w->setAttribute("name", attribute);
    sprintf(tmp,"%g",value);
    w->setAttribute("value", tmp);
    w->closeTag("attribute");
}
