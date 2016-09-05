//
//  ConversationGraph.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 5/31/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
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

#include "A4Script.h"
#include "A4Game.h"
#include "ConversationGraph.h"
#include "A4Object.h"
#include "A4Character.h"

// if you don't say anything in 30 seconds, they complain:
#define CONVERSATION_TIME_OUT   1500

ConversationGraph::ConversationGraph()
{
}

ConversationGraph::ConversationGraph(XMLNode *xml)
{
    std::vector<XMLNode *> *state_xml_l = xml->get_children("state");
    
    for(XMLNode *state_xml:*state_xml_l) {
        m_states.push_back(new ConversationGraphState(state_xml));
    }
    
    delete state_xml_l;
}


ConversationGraph::~ConversationGraph()
{
    for(ConversationGraphState *s:m_states) delete s;
    m_states.clear();
}


void ConversationGraph::addConversationGraphTransition(XMLNode *xml)
{
    ConversationGraphTransition *t = new ConversationGraphTransition();
    
    if (xml->get_attribute("actor")!=0 &&
        strcmp(xml->get_attribute("actor"),"self")==0) t->m_trigger_actor = CGT_ACTOR_SELF;
    if (xml->get_attribute("performative")!=0) {
        if (strcmp(xml->get_attribute("performative"),"hi")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_HI;
        else if (strcmp(xml->get_attribute("performative"),"bye")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_BYE;
        else if (strcmp(xml->get_attribute("performative"),"trade")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_TRADE;
        else if (strcmp(xml->get_attribute("performative"),"endtrade")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_END_TRADE;
        else if (strcmp(xml->get_attribute("performative"),"ask")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_ASK;
        else if (strcmp(xml->get_attribute("performative"),"inform")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_INFORM;
        else if (strcmp(xml->get_attribute("performative"),"timeout")==0) t->m_trigger_performative = A4_TALK_PERFORMATIVE_TIMEOUT;
    }
    // both "topic" or "keyword" are the same:
    if (xml->get_attribute("keyword")!=0) {
        t->m_trigger_keyword = new Symbol(xml->get_attribute("keyword"));
    } else if (xml->get_attribute("topic")!=0) {
        t->m_trigger_keyword = new Symbol(xml->get_attribute("topic"));
    }
    if (xml->get_attribute("state")!=0) {
        t->m_state = new Symbol(xml->get_attribute("state"));
    }
    if (xml->get_attribute("consume")!=0 &&
        strcmp(xml->get_attribute("consume"),"false")==0) t->m_consume = false;
    for(XMLNode *script_xml:*(xml->get_children())) {
        t->m_effects.push_back(new A4Script(script_xml));
    }

    Symbol *from = new Symbol(xml->get_attribute("from"));
    addConversationGraphTransition(from, t);
    delete from;
}


void ConversationGraph::addConversationGraphTransition(Symbol *state, ConversationGraphTransition *t)
{
    for(ConversationGraphState *s:m_states) {
        if (s->m_name->cmp(state)) {
            s->addConversationGraphTransition(t);
            return;
        }
    }
    output_debug_message("Adding conversation graph transition to unexistent state '%s' (creting it...)\n",state->get());
    m_states.push_back(new ConversationGraphState(state));
}


void ConversationGraph::saveToXML(class XMLwriter *w)
{
    w->openTag("conversationGraph");
    for(ConversationGraphState *s:m_states) s->saveToXML(w);
    w->closeTag("conversationGraph");
}


ConversationGraphState *ConversationGraph::getState(const char *state)
{
    for(ConversationGraphState *s:m_states) {
        if (s->m_name->cmp(state)) return s;
    }
    return 0;
}


ConversationGraphState *ConversationGraph::getState(Symbol *state)
{
    for(ConversationGraphState *s:m_states) {
        if (s->m_name->cmp(state)) return s;
    }
    return 0;
}


ConversationGraphState::ConversationGraphState(Symbol *name)
{
    m_name = new Symbol(name);
}



ConversationGraphState::ConversationGraphState(XMLNode *xml)
{
    std::vector<XMLNode *> *transition_xml_l = xml->get_children("transition");
    
    m_name = new Symbol(xml->get_attribute("name"));
    for(XMLNode *transition_xml:*transition_xml_l) {
        m_transitions.push_back(new ConversationGraphTransition(transition_xml));
    }
    
    delete transition_xml_l;
}


ConversationGraphState::~ConversationGraphState()
{
    delete m_name;
    m_name = 0;
    for(ConversationGraphTransition *t:m_transitions) delete t;
    m_transitions.clear();
}


void ConversationGraphState::addConversationGraphTransition(ConversationGraphTransition *t)
{
    std::vector<ConversationGraphTransition *> toDelete;
    for(ConversationGraphTransition *t2:m_transitions) {
        if (t2->m_trigger_actor == t->m_trigger_actor &&
            t2->m_trigger_performative == t->m_trigger_performative) {
            if ((t->m_trigger_keyword==0 && t2->m_trigger_keyword==0) ||
                (t->m_trigger_keyword!=0 && t->m_trigger_keyword->cmp(t2->m_trigger_keyword))) {
                // same transition, replace the old one:
                toDelete.push_back(t2);
            }
        }
    }
    for(ConversationGraphTransition *t2:toDelete) m_transitions.remove(t2);

    m_transitions.push_back(t);
}


void ConversationGraphState::saveToXML(class XMLwriter *w)
{
    w->openTag("state");
    w->setAttribute("name", m_name->get());
    for(ConversationGraphTransition *t:m_transitions) t->saveToXML(w);
    w->closeTag("state");
}



ConversationGraphTransition::ConversationGraphTransition()
{
    m_trigger_actor = CGT_ACTOR_OTHER;
    m_trigger_performative = -1;
    m_trigger_keyword = 0;
    m_state = 0;
    m_consume = true;
}


ConversationGraphTransition::ConversationGraphTransition(XMLNode *xml)
{
    m_trigger_actor = CGT_ACTOR_OTHER;
    if (xml->get_attribute("actor")!=0 &&
        strcmp(xml->get_attribute("actor"),"self")==0) m_trigger_actor = CGT_ACTOR_SELF;

    m_trigger_performative = -1;
    if (xml->get_attribute("performative")!=0) {
        if (strcmp(xml->get_attribute("performative"),"hi")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_HI;
        else if (strcmp(xml->get_attribute("performative"),"bye")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_BYE;
        else if (strcmp(xml->get_attribute("performative"),"trade")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_TRADE;
        else if (strcmp(xml->get_attribute("performative"),"endtrade")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_END_TRADE;
        else if (strcmp(xml->get_attribute("performative"),"ask")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_ASK;
        else if (strcmp(xml->get_attribute("performative"),"inform")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_INFORM;
        else if (strcmp(xml->get_attribute("performative"),"timeout")==0) m_trigger_performative = A4_TALK_PERFORMATIVE_TIMEOUT;
    }
    
    if (xml->get_attribute("keyword")!=0) {
        m_trigger_keyword = new Symbol(xml->get_attribute("keyword"));
    } else {
        m_trigger_keyword = 0;
    }
    
    if (xml->get_attribute("state")!=0) {
        m_state = new Symbol(xml->get_attribute("state"));
    } else {
        m_state = 0;
    }
    
    m_consume = true;
    if (xml->get_attribute("consume")!=0 &&
        strcmp(xml->get_attribute("consume"),"false")==0) m_consume = false;
    
    for(XMLNode *script_xml:*(xml->get_children())) {
        m_effects.push_back(new A4Script(script_xml));
    }
    
}


ConversationGraphTransition::~ConversationGraphTransition()
{
    if (m_trigger_keyword!=0) delete m_trigger_keyword;
    if (m_state!=0) delete m_state;
    for(A4Script *s:m_effects) delete s;
    m_effects.clear();
}


bool ConversationGraphTransition::match(int actor, SpeechAct *sa)
{
    if (m_trigger_actor!=-1 && actor!=m_trigger_actor) return false;
    if (m_trigger_performative!=-1 && sa->performative!=m_trigger_performative) return false;
    if (m_trigger_keyword!=0 && !m_trigger_keyword->cmp(sa->keyword)) return false;
    return true;
}


bool ConversationGraphTransition::moreSpecificThan(ConversationGraphTransition *t)
{
    if (t->m_trigger_actor!=-1 && t->m_trigger_actor!=m_trigger_actor) return false;
    if (t->m_trigger_performative!=-1 && t->m_trigger_performative!=m_trigger_performative) return false;
    if (t->m_trigger_keyword!=0 && !t->m_trigger_keyword->cmp(m_trigger_keyword)) return false;
    return true;
}


void ConversationGraphTransition::saveToXML(class XMLwriter *w)
{
    const char *actorNames[]={"self","other"};
    w->openTag("transition");
    if (m_trigger_actor!=-1) w->setAttribute("actor", actorNames[m_trigger_actor]);
    if (m_trigger_performative!=-1) w->setAttribute("performative", A4Game::performativeNames[m_trigger_performative]);
    if (m_trigger_keyword!=0) w->setAttribute("keyword", m_trigger_keyword->get());
    if (m_state!=0) w->setAttribute("state", m_state->get());
    w->setAttribute("consume", m_consume ? "true":"false");
    for(A4Script *s:m_effects) s->saveToXML(w);
    w->closeTag("transition");
}


// ---- ---- ---- ---- ---- ---- ---- ----

ConversationGraphInstance::ConversationGraphInstance(ConversationGraph *g, A4Character *oc)
{
    m_other_character = oc;
    m_graph = g;
    m_currentState = m_graph->getState("none");
    m_timer = 0;
    m_script_queue = 0;
    m_other_moved_away = false;
}


ConversationGraphInstance::~ConversationGraphInstance()
{
    m_other_character = 0;
    m_graph = 0;
    m_currentState = 0;
    for(SpeechAct *sa:m_other_queue) delete sa;
    m_other_queue.clear();
    for(SpeechAct *sa:m_self_queue) delete sa;
    m_self_queue.clear();
    if (m_script_queue!=0) delete m_script_queue;
    m_script_queue = 0;
}


void ConversationGraphInstance::input(SpeechAct *sa)
{
    m_other_queue.push_back(sa);
}

void ConversationGraphInstance::inputFromSelf(SpeechAct *sa)
{
    m_self_queue.push_back(sa);
}


bool ConversationGraphInstance::cycle(A4Character *c, A4Game *game)
{
    if (m_script_queue==0) {
        // process input:
        if (m_other_queue.empty() && m_self_queue.empty()) {
            // nothing to do.
            if (m_other_moved_away) return false;
        } else {
            int speaker = CGT_ACTOR_OTHER;
            SpeechAct *sa = 0;
            if (!m_self_queue.empty()) {
                sa = m_self_queue.front();
                speaker = CGT_ACTOR_SELF;
            } else {
                sa = m_other_queue.front();
            }
            ConversationGraphTransition *best = 0;
            for(ConversationGraphTransition *t:m_currentState->m_transitions) {
                if (t->match(speaker, sa)) {
                    if (best==0) {
                        best = t;
                    } else {
                        if (t->moreSpecificThan(best)) best = t;
                    }
                }
            }
            if (best!=0) {
                output_debug_message("ConversationGraphInstance %p (character %i): %s -> (%i,%s) -> %s\n", this, c->getID(),
                                     m_currentState->m_name->get(), sa->performative, sa->keyword,
                                     (best->m_state==0 ? "-":best->m_state->get()));
                // execute the transition:
                if (best->m_consume) {
                    m_self_queue.remove(sa);
                    m_other_queue.remove(sa);
                    delete sa;
                }
                if (best->m_state!=0) m_currentState = m_graph->getState(best->m_state);
                if (!best->m_effects.empty()) {
                    m_script_queue = new A4ScriptExecutionQueue(c, c->getMap(), game, m_other_character);
                    for(A4Script *s:best->m_effects) {
                        m_script_queue->scripts.push_back(new A4Script(s));
                    }
                }
                m_timer = 0;
            } else {
                // input without transition!! just ignore it...
                output_debug_message("ConversationGraphInstance %p (character %i): %s -> ignoring speech act (%i,%s)\n", this, c->getID(),
                                     m_currentState->m_name->get(), sa->performative, sa->keyword);
                m_self_queue.remove(sa);
                m_other_queue.remove(sa);
                delete sa;
            }
        }
        // time out only happens once (so it's '==' instead of '>=')
        if (m_timer==CONVERSATION_TIME_OUT) {
            input(new SpeechAct(A4_TALK_PERFORMATIVE_TIMEOUT, 0, 0));
        }
    } else {
        // execute the scripts:
        while(true) {
            A4Script *s = m_script_queue->scripts.front();
            int retval = s->execute((m_script_queue->object == 0 ? c:m_script_queue->object),
                                    (m_script_queue->map == 0 ? c->getMap():m_script_queue->map),
                                    (m_script_queue->game == 0 ? game:m_script_queue->game),
                                    m_script_queue->otherCharacter);
            if (retval==SCRIPT_FINISHED) {
                m_script_queue->scripts.pop_front();
                delete s;
                if (m_script_queue->scripts.empty()) {
                    delete m_script_queue;
                    m_script_queue = 0;
                    break;
                }
            } else if (retval==SCRIPT_NOT_FINISHED) {
                break;
            } else if (retval==SCRIPT_FAILED) {
                delete m_script_queue;
                m_script_queue = 0;
                break;
            }
        }
    }
    
    if (c->isTalkingIdle() && m_other_character->isTalkingIdle())m_timer++;
    return true;
}


bool ConversationGraphInstance::willAcceptSpeechAct(SpeechAct *sa)
{
    int speaker = CGT_ACTOR_OTHER;
    ConversationGraphTransition *best = 0;
    for(ConversationGraphTransition *t:m_currentState->m_transitions) {
        if (t->match(speaker, sa)) {
            if (best==0) {
                best = t;
            } else {
                if (t->moreSpecificThan(best)) best = t;
            }
        }
    }
    if (best==0) return false;
    for(A4Script *effect:best->m_effects) {
        if (effect->m_type == A4_SCRIPT_TALK && effect->angry) return false;
    }
    
    return true;
}


bool ConversationGraphInstance::willAcceptSpeechActFromSelf(SpeechAct *sa)
{
    int speaker = CGT_ACTOR_SELF;
    ConversationGraphTransition *best = 0;
    for(ConversationGraphTransition *t:m_currentState->m_transitions) {
        if (t->match(speaker, sa)) {
            if (best==0) {
                best = t;
            } else {
                if (t->moreSpecificThan(best)) best = t;
            }
        }
    }
    if (best==0) return false;
    for(A4Script *effect:best->m_effects) {
        if (effect->m_type == A4_SCRIPT_TALK && effect->angry) return false;
    }
    
    return true;
}


