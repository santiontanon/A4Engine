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
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
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

#include "sound.h"
#include "keyboardstate.h"
#include "BB2D.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "XMLparser.h"
#include "XMLwriter.h"
#include "ExpressionParser.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Game.h"
#include "A4Object.h"
#include "A4Character.h"
#include "A4Map.h"


A4EventRule::A4EventRule(XMLNode *xml)
{
	char *event_name = xml->get_attribute("event");
    m_item = 0;
    m_variable = 0;
    m_value = 0;
    m_character = 0;
    m_hit = -1;
    m_angry = -1;
    m_performative = -1;
    m_spell = -1;

    if (event_name==0) {
        output_debug_message("A4EventRule: no event in rule!\n");
        exit(1);
    } else {
        if (strcmp(event_name,"use")==0) {
            m_event = A4_EVENT_USE;
        } else if (strcmp(event_name,"pickup")==0) {
            m_event = A4_EVENT_PICKUP;
        } else if (strcmp(event_name,"activate")==0) {
            m_event = A4_EVENT_ACTIVATE;
        } else if (strcmp(event_name,"deactivate")==0) {
            m_event = A4_EVENT_DEACTIVATE;
        } else if (strcmp(event_name,"drop")==0) {
            m_event = A4_EVENT_DROP;
        } else if (strcmp(event_name,"open")==0) {
            m_event = A4_EVENT_OPEN;
        } else if (strcmp(event_name,"close")==0) {
            m_event = A4_EVENT_CLOSE;
        } else if (strcmp(event_name,"push")==0) {
            m_event = A4_EVENT_PUSH;
        } else if (strcmp(event_name,"timer")==0) {
            m_event = A4_EVENT_TIMER;
            m_time = atoi(xml->get_attribute("time"));
            if (xml->get_attribute("period")!=0) {
                m_period = atoi(xml->get_attribute("period"));
            } else {
                m_period = 0;
            }
        } else if (strcmp(event_name,"receive")==0) {
            m_event = A4_EVENT_RECEIVE;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"interact")==0) {
            m_event = A4_EVENT_INTERACT;
        } else if (strcmp(event_name,"start")==0) {
            m_event = A4_EVENT_START;
        } else if (strcmp(event_name,"end")==0) {
            m_event = A4_EVENT_END;
        } else if (strcmp(event_name,"story_state")==0) {
            char *scope_name = xml->get_attribute("scope");
            if (scope_name!=0) {
                // it's a story state rule:
                m_event = A4_EVENT_STORYSTATE;
                if (strcmp(scope_name, "game")==0) m_ss_scope = A4_STORYSTATE_GAME;
                else if (strcmp(scope_name, "map")==0) m_ss_scope = A4_STORYSTATE_MAP;
                else if (strcmp(scope_name, "character")==0) m_ss_scope = A4_STORYSTATE_OBJECT;
                else if (strcmp(scope_name, "object")==0) m_ss_scope = A4_STORYSTATE_OBJECT;
                else {
                    output_debug_message("A4EventRule: unrecognized scope '%s'!\n", scope_name);
                    exit(1);
                }
                char *variable_name = xml->get_attribute("variable");
                char *value_name = xml->get_attribute("value");
                m_variable = new char[strlen(variable_name)+1];
                strcpy(m_variable,variable_name);
                m_value = new char[strlen(value_name)+1];
                strcpy(m_value,value_name);
            } else {
                output_debug_message("A4EventRule: no scope in story state rule!\n");
                exit(1);
            }
        } else if (strcmp(event_name,"action_take")==0) {
            m_event = A4_EVENT_ACTION_TAKE;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_drop")==0) {
            m_event = A4_EVENT_ACTION_DROP;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_drop_gold")==0) {
            m_event = A4_EVENT_ACTION_DROP_GOLD;
        } else if (strcmp(event_name,"action_use")==0) {
            m_event = A4_EVENT_ACTION_USE;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_equip")==0) {
            m_event = A4_EVENT_ACTION_EQUIP;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_unequip")==0) {
            m_event = A4_EVENT_ACTION_UNEQUIP;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_interact")==0) {
            m_event = A4_EVENT_ACTION_INTERACT;
            char *tmp = xml->get_attribute("item");
            if (tmp==0) {
                m_item = 0;
            } else {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else if (strcmp(event_name,"action_talk")==0) {
            m_event = A4_EVENT_ACTION_TALK;
            char *tmp = xml->get_attribute("performative");
            if (tmp!=0) {
                if (strcmp(tmp,"hi")==0) m_performative = A4_TALK_PERFORMATIVE_HI;
                if (strcmp(tmp,"bye")==0) m_performative = A4_TALK_PERFORMATIVE_BYE;
                if (strcmp(tmp,"ask")==0) m_performative = A4_TALK_PERFORMATIVE_ASK;
                if (strcmp(tmp,"inform")==0) m_performative = A4_TALK_PERFORMATIVE_INFORM;
                if (strcmp(tmp,"trade")==0) m_performative = A4_TALK_PERFORMATIVE_TRADE;
                if (strcmp(tmp,"endtrade")==0) m_performative = A4_TALK_PERFORMATIVE_END_TRADE;
            }
            tmp = xml->get_attribute("angry");
            if (tmp!=0 && strcmp(tmp,"true")==0) {
                m_angry = 1;
            } else {
                m_angry = 0;
            }
        } else if (strcmp(event_name,"action_attack")==0) {
            m_event = A4_EVENT_ACTION_ATTACK;
            char *tmp = xml->get_attribute("character");
            if (tmp!=0) {
                m_character = new char[strlen(tmp)+1];
                strcpy(m_character, tmp);
            }
            tmp = xml->get_attribute("hit");
            if (tmp!=0 && strcmp(tmp,"true")==0) {
                m_hit = 1;
            } else {
                m_hit = 0;
            }
        } else if (strcmp(event_name,"action_spell")==0) {
            m_event = A4_EVENT_ACTION_SPELL;
            char *tmp = xml->get_attribute("spell");
            if (tmp!=0) {
                for(int i = 0;i<A4_N_SPELLS;i++) {
                    if (strcmp(tmp, A4Game::spellNames[i])==0) {
                        m_spell = i;
                    }
                }
            }
        } else if (strcmp(event_name,"action_give")==0) {
            m_event = A4_EVENT_ACTION_GIVE;
            char *tmp = xml->get_attribute("item");
            if (tmp!=0) {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
            tmp = xml->get_attribute("character");
            if (tmp!=0) {
                m_character = new char[strlen(tmp)+1];
                strcpy(m_character, tmp);
            }
        } else if (strcmp(event_name,"action_sell")==0) {
            m_event = A4_EVENT_ACTION_SELL;
            char *tmp = xml->get_attribute("item");
            if (tmp!=0) {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
            tmp = xml->get_attribute("character");
            if (tmp!=0) {
                m_character = new char[strlen(tmp)+1];
                strcpy(m_character, tmp);
            }
        } else if (strcmp(event_name,"action_buy")==0) {
            m_event = A4_EVENT_ACTION_BUY;
            char *tmp = xml->get_attribute("item");
            if (tmp!=0) {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
            tmp = xml->get_attribute("character");
            if (tmp!=0) {
                m_character = new char[strlen(tmp)+1];
                strcpy(m_character, tmp);
            }
        } else if (strcmp(event_name,"action_chop")==0) {
            m_event = A4_EVENT_ACTION_CHOP;
            char *tmp = xml->get_attribute("item");
            if (tmp!=0) {
                m_item = new char[strlen(tmp)+1];
                strcpy(m_item, tmp);
            }
        } else {
            output_debug_message("A4EventRule: event not recognized '%s'\n", event_name);
            exit(1);
        }
    }
    char *once_text = xml->get_attribute("once");
    if (once_text!=0 && strcmp(once_text,"true")==0) {
        m_once = true;
    } else {
        m_once = false;
    }

	for(XMLNode *script_xml:*(xml->get_children())) {
		m_effects.push_back(new A4Script(script_xml));
	}
    
    m_start_time = -1;
    m_executed = false;
}


A4EventRule::A4EventRule(A4EventRule *rule)
{
    m_event = rule->m_event;
    m_time = rule->m_time;
    m_period = rule->m_period;
    m_start_time = rule->m_start_time;
    m_once = rule->m_once;
    m_executed = rule->m_executed;
    if (rule->m_variable==0) {
        m_variable = 0;
    } else {
        m_variable = new char[strlen(rule->m_variable)+1];
        strcpy(m_variable, rule->m_variable);
    }
    if (rule->m_value==0) {
        m_value = 0;
    } else {
        m_value = new char[strlen(rule->m_value)+1];
        strcpy(m_value, rule->m_value);
    }
    if (rule->m_item==0) {
        m_item = 0;
    } else {
        m_item = new char[strlen(rule->m_item)+1];
        strcpy(m_item, rule->m_item);
    }
    if (rule->m_character==0) {
        m_character = 0;
    } else {
        m_character = new char[strlen(rule->m_character)+1];
        strcpy(m_character, rule->m_character);
    }
    m_hit = rule->m_hit;
    m_angry = rule->m_angry;
    m_performative = rule->m_performative;
    m_spell = rule->m_spell;
    for(A4Script *s:rule->m_effects) {
        m_effects.push_back(new A4Script(s));
    }
}


A4EventRule::A4EventRule(int event)
{
    m_event = event;
    m_time = 0;
    m_period = 0;
    m_start_time = 01;
    m_once = false;
    m_executed = false;
    m_variable = 0;
    m_value = 0;
    m_item = 0;
    m_character = 0;
    m_hit = -1;
    m_angry = -1;
    m_performative = -1;
    m_spell = -1;
}


A4EventRule::A4EventRule(int event, A4Script *script, bool once)
{
	m_event = event;
	m_time = 0;
	m_period = 0;
    m_start_time = -1;
    m_once = once;
    m_executed = false;
	m_effects.push_back(script);
    m_variable = 0;
    m_value = 0;
    m_item = 0;
    m_character = 0;
    m_hit = -1;
    m_angry = -1;
    m_performative = -1;
    m_spell = -1;
}


A4EventRule::A4EventRule(int event, int a_time, int period,A4Script *script, bool once)
{
	m_event = event;
	m_time = a_time;
	m_period = period;
    m_start_time = -1;
    m_once = once;
    m_executed = false;
	m_effects.push_back(script);
    m_variable = 0;
    m_value = 0;
    m_item = 0;
    m_character = 0;
    m_hit = -1;
    m_angry = -1;
    m_performative = -1;
    m_spell = -1;
}


void A4EventRule::addScript(class A4Script *s)
{
    m_effects.push_back(s);
}



A4EventRule::~A4EventRule()
{
	for(A4Script *s:m_effects) delete s;
	m_effects.clear();
    if (m_variable!=0) delete m_variable;
    m_variable = 0;
    if (m_value!=0) delete m_value;
    m_value = 0;
    if (m_item!=0) delete m_item;
    m_item = 0;
    if (m_character!=0) delete m_character;
    m_character = 0;
}


int A4EventRule::executeEffects(A4Object *p, class A4Map *map, class A4Game *game, A4Character *otherCharacter)
{
    if (m_once && m_executed) return SCRIPT_FINISHED;
    m_executed = true;
	int retValue = SCRIPT_FINISHED;
    A4ScriptExecutionQueue *seq = 0;
	for(A4Script *s:m_effects) {
        if (seq==0) {
            s->reset();
            retValue = s->execute(p, map, game, otherCharacter);
            if (retValue == SCRIPT_FINISHED) {
                // good, do nothing
            } else if (retValue == SCRIPT_NOT_FINISHED) {
                // script needs more time, create an script queue
                seq = new A4ScriptExecutionQueue(p, map, game, otherCharacter);
                seq->scripts.push_back(new A4Script(s));
                if (p!=0 ) {
                    p->addScriptQueue(seq);
                } else if (map!=0) {
                    map->addScriptQueue(seq);
                } else {
                    game->addScriptQueue(seq);
                }
            } else {
                // failed, stop the script
                break;
            }
        } else {
            s->reset();
            seq->scripts.push_back(new A4Script(s));
        }
	}
	return retValue;
}

int A4EventRule::execute(class A4Object *o, class A4Map *map, class A4Game *game, class A4Character *otherCharacter)
{
    if (m_once && m_executed) return SCRIPT_FINISHED;
    
	// check if the condition is met first:
    switch(m_event) {
        case A4_EVENT_TIMER:
            {
                if (m_start_time<0) m_start_time = game->getCycle();
                int t = game->getCycle() - m_start_time;
                if (m_period!=0) {
                    if ((t%m_period) == m_time) return executeEffects(o, map, game, otherCharacter);
                } else {
                    if (t == m_time) return executeEffects(o, map, game, otherCharacter);
                }
            }
            break;
        case A4_EVENT_STORYSTATE:
            switch(m_ss_scope) {
                case A4_STORYSTATE_GAME:
                    {
                        char *v = game->getStoryStateVariable(m_variable);
                        if (v!=0 && strcmp(v,m_value)==0) {
                            return executeEffects(o, map, game, otherCharacter);
                        }
                    }
                    break;
                case A4_STORYSTATE_MAP:
                    {
                        char *v = map->getStoryStateVariable(m_variable);
                        if (v!=0 && strcmp(v,m_value)==0) return executeEffects(o, map, game, otherCharacter);
                    }
                    break;
                case A4_STORYSTATE_OBJECT:
                    {
                        char *v = o->getStoryStateVariable(m_variable);
                        if (v!=0 && strcmp(v,m_value)==0) return executeEffects(o, map, game, otherCharacter);
                    }
                    break;
            }
            break;
    }

	return SCRIPT_FINISHED;
}


void A4EventRule::saveToXML(XMLwriter *w)
{
    if (m_once && m_executed) return;   // it has already been executed, so, no need to save it!
    
    // event rules:
    w->openTag("eventRule");
    if (m_event==A4_EVENT_USE) w->setAttribute("event", "use");
    if (m_event==A4_EVENT_PICKUP) w->setAttribute("event", "pickup");
    if (m_event==A4_EVENT_ACTIVATE) w->setAttribute("event", "activate");
    if (m_event==A4_EVENT_DEACTIVATE) w->setAttribute("event", "deactivate");
    if (m_event==A4_EVENT_DROP) w->setAttribute("event", "drop");
    if (m_event==A4_EVENT_EQUIP) w->setAttribute("event", "equip");
    if (m_event==A4_EVENT_UNEQUIP) w->setAttribute("event", "unequip");
    if (m_event==A4_EVENT_OPEN) w->setAttribute("event", "open");
    if (m_event==A4_EVENT_CLOSE) w->setAttribute("event", "close");
    if (m_event==A4_EVENT_PUSH) w->setAttribute("event", "push");
    if (m_event==A4_EVENT_TIMER) {
        w->setAttribute("event", "timer");
        w->setAttribute("time", m_time);
        if (m_period!=0) w->setAttribute("period", m_period);
    }
    if (m_event==A4_EVENT_INTERACT) w->setAttribute("event", "interact");
    if (m_event==A4_EVENT_RECEIVE) {
        w->setAttribute("event", "receive");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_START) w->setAttribute("event", "start");
    if (m_event==A4_EVENT_END) w->setAttribute("event", "end");
    
    if (m_event==A4_EVENT_STORYSTATE) {
        // story state rule:
        w->setAttribute("event", "story_state");
        if (m_ss_scope==A4_STORYSTATE_GAME) w->setAttribute("scope", "game");
        if (m_ss_scope==A4_STORYSTATE_MAP) w->setAttribute("scope", "map");
        if (m_ss_scope==A4_STORYSTATE_OBJECT) w->setAttribute("scope", "object");
        w->setAttribute("variable", m_variable);
        w->setAttribute("value", m_value);
        if (m_once)w->setAttribute("once", "true");
        for(A4Script *s:m_effects) {
            s->saveToXML(w);
        }
    }
    
    if (m_event==A4_EVENT_ACTION_TAKE) {
        w->setAttribute("event", "action_take");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_DROP) {
        w->setAttribute("event", "action_drop");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_DROP_GOLD) {
        w->setAttribute("event", "action_drop_gold");
    }
    if (m_event==A4_EVENT_ACTION_USE) {
        w->setAttribute("event", "action_use");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_EQUIP) {
        w->setAttribute("event", "action_equip");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_UNEQUIP) {
        w->setAttribute("event", "action_unequip");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_INTERACT) {
        w->setAttribute("event", "action_interact");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_event==A4_EVENT_ACTION_TALK) {
        w->setAttribute("event", "action_talk");
        if (m_performative==A4_TALK_PERFORMATIVE_HI) w->setAttribute("performative", "hi");
        if (m_performative==A4_TALK_PERFORMATIVE_BYE) w->setAttribute("performative", "bye");
        if (m_performative==A4_TALK_PERFORMATIVE_ASK) w->setAttribute("performative", "ask");
        if (m_performative==A4_TALK_PERFORMATIVE_INFORM) w->setAttribute("performative", "inform");
        if (m_performative==A4_TALK_PERFORMATIVE_TRADE) w->setAttribute("performative", "trade");
        if (m_performative==A4_TALK_PERFORMATIVE_END_TRADE) w->setAttribute("performative", "endtrade");
        if (m_angry==0) w->setAttribute("angry", "false");
        if (m_angry==1) w->setAttribute("angry", "true");
    }
    if (m_event==A4_EVENT_ACTION_ATTACK) {
        w->setAttribute("event", "action_attack");
        if (m_character!=0) w->setAttribute("character", m_character);
        if (m_hit==0) w->setAttribute("hit", "false");
        if (m_hit==1) w->setAttribute("hit", "true");
    }
    if (m_event==A4_EVENT_ACTION_SPELL) {
        w->setAttribute("event", "action_spell");
        if (m_spell>=0) w->setAttribute("spell", A4Game::spellNames[m_spell]);
    }
    if (m_event==A4_EVENT_ACTION_GIVE) {
        w->setAttribute("event", "action_give");
        if (m_item!=0) w->setAttribute("item", m_item);
        if (m_character!=0) w->setAttribute("character", m_character);
    }
    if (m_event==A4_EVENT_ACTION_SELL) {
        w->setAttribute("event", "action_sell");
        if (m_item!=0) w->setAttribute("item", m_item);
        if (m_character!=0) w->setAttribute("character", m_character);
    }
    if (m_event==A4_EVENT_ACTION_BUY) {
        w->setAttribute("event", "action_buy");
        if (m_item!=0) w->setAttribute("item", m_item);
        if (m_character!=0) w->setAttribute("character", m_character);
    }
    if (m_event==A4_EVENT_ACTION_CHOP) {
        w->setAttribute("event", "action_chop");
        if (m_item!=0) w->setAttribute("item", m_item);
    }
    if (m_once)w->setAttribute("once", "true");
    for(A4Script *s:m_effects) {
        s->saveToXML(w);
    }        
    w->closeTag("eventRule");
}

