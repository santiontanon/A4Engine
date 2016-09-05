//
//  A4AI.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

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
#include <algorithm>
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
#include "A4AICharacter.h"
#include "A4Vehicle.h"

#include "A4Behavior.h"
#include "ConversationGraph.h"

#include "A4AI.h"


#define PERCEIVED_ACTION_ACTIVATION     100
#define PERCEIVED_WARP_ACTIVATION       200


Symbol *A4AI::s_object_symbol = 0;
Symbol *A4AI::s_inventory_symbol = 0;
Symbol *A4AI::s_location_symbol = 0;
Symbol *A4AI::s_bridge_symbol = 0;
Symbol *A4AI::s_action_take = 0;
Symbol *A4AI::s_action_drop = 0;
Symbol *A4AI::s_action_die = 0;
Symbol *A4AI::s_action_attack = 0;
Symbol *A4AI::s_action_cast = 0;
Symbol *A4AI::s_action_interact = 0;
Symbol *A4AI::s_action_embark = 0;
Symbol *A4AI::s_action_disembark = 0;
Symbol *A4AI::s_action_talk_performatives[A4_TALK_N_PERFORMATIVES];
Symbol *A4AI::s_action_give = 0;
Symbol *A4AI::s_action_buy = 0;
Symbol *A4AI::s_friendly_symbol = 0;
Symbol *A4AI::s_unfriendly_symbol = 0;

A4AI::A4AI(A4AICharacter *c, int sightRadius)
{
    // 3000 = 50*60 (50 fps * 60 seconds: a character remembers something if it has been activated for 1 minute continuously)
    m_memory = new AIMemory(3000);
//    m_memory->addUnfreezeableFunctor(s_object_symbol);
    m_character = c;
    m_period = 1;   // perception is run every cycle
    m_cycle = 0;
    m_sightRadius = sightRadius;
    m_navigationBuffer_lastUpdated = -1;
    m_navigationBuffer_size = 0;
    m_navigationBuffer = 0;
    m_navigationBuffer_bridges = 0;
    m_navigationBuffer_x = m_navigationBuffer_y = 0;
    m_navigationBuffer_map = 0;
    m_pathfinding_n_targets = m_pathfinding_n_targets_old = 0;
    m_pathfinding_closed = 0;
    m_pathfinding_open = 0;
    m_pathfinding_result_x = -1;
    m_pathfinding_result_y = -1;
    m_pathfinding_result_offset_x = 0;
    m_pathfinding_result_offset_y = 0;
    m_pathfinding_result_priority = -1;
    m_pathFinding_lastUpdated = -1;
    m_pathFinding_iterations = 0;
    m_nextPathFindingTimer = 0;
    m_n_friendly = m_n_unfriendly = 0;
    m_conversationGraph = 0;
    m_n_all_objects_buffer = 0;
    m_is_a_pattern = 0;
    m_is_friendly_sort_pattern = 0;
    m_is_unfriendly_sort_pattern = 0;
}


A4AI::~A4AI() {
    if (m_is_a_pattern!=0) delete m_is_a_pattern;
    m_is_a_pattern = 0;
    if (m_is_friendly_sort_pattern!=0) delete m_is_friendly_sort_pattern;
    m_is_friendly_sort_pattern = 0;
    if (m_is_unfriendly_sort_pattern!=0) delete m_is_unfriendly_sort_pattern;
    m_is_unfriendly_sort_pattern = 0;
    for(ConversationGraphInstance *cgi:m_conversations) delete cgi;
    m_conversations.clear();
    if (m_conversationGraph!=0) delete m_conversationGraph;
    m_conversationGraph = 0;
    delete m_memory;
    m_memory = 0;
    if (m_navigationBuffer!=0) delete m_navigationBuffer;
    m_navigationBuffer = 0;
    if (m_navigationBuffer_bridges!=0) delete m_navigationBuffer_bridges;
    m_navigationBuffer_bridges = 0;
    m_navigationBuffer_map = 0;
    for(InferenceRule *r:m_inferenceRules) delete r;
    m_inferenceRules.clear();
    for(A4Behavior *b:m_behaviors) delete b;
    m_behaviors.clear();
    if (m_pathfinding_closed!=0) delete m_pathfinding_closed;
    if (m_pathfinding_open!=0) delete m_pathfinding_open;
    for(A4Script *s:m_pendingTalk) delete s;
    m_pendingTalk.clear();
}


WME *A4AI::addShortTermWME(WME *wme)
{
    return m_memory->addShortTermWME(wme);
}


WME *A4AI::addLongTermWME(WME *wme)
{
    return m_memory->addLongTermWME(wme);
}


void A4AI::cycle(A4Game *game)
{
    // 1) perception & 2) fact checking:
    perception(game);
    
    // 3) reasoning:
    for(InferenceRule *r:m_inferenceRules) {
        if ((double(rand()) / RAND_MAX) < r->getFrequency()) r->execute(m_memory, m_character, m_character->getMap(), game);
    }

    // 4) memory updates:
    m_memory->cycle(m_period);
    
    // (5.0) cache friendly/unfriendly lists before running the behaviors:
    {
        m_n_friendly = 0;
        m_n_unfriendly = 0;
        std::vector<WME *> *l = m_memory->retrieveByFunctor(s_object_symbol);
        for(WME *wme:*l) {
            if (m_is_a_pattern==0) {
                m_is_a_pattern = new WME(AIMemory::s_is_a_symbol,
                                         wme->getParameter(0), WME_PARAMETER_INTEGER,
                                         WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
            } else {
                m_is_a_pattern->setParameter(0, wme->getParameter(0));
            }
            WME *w2 = m_memory->retrieveFirstBySubsumption(m_is_a_pattern);
            if (w2==0 || w2->getParameterType(1)!=WME_PARAMETER_SORT) continue;
            Sort *sort = w2->getParameter(1).m_sort;
            if (isUnfriendly(sort)) {
                m_unfriendly_cache[m_n_unfriendly] = wme;
                m_n_unfriendly++;
            } else if (isFriendly(sort)) {
                m_friendly_cache[m_n_friendly] = wme;
                m_n_friendly++;
            }
        }
        delete l;
    }

    // 5: conversations:
    {
        std::vector<ConversationGraphInstance *> toDelete;
        for(ConversationGraphInstance *cgi:m_conversations) {
            bool found = false;
            for(int i = 0;i<m_n_all_objects_buffer;i++) {
                if (m_all_objects_buffer[i] == cgi->m_other_character) found = true;
            }
            if (!found) {
                // we cannot see the character, see if it is dead, or just moved away:
                if (game->contains(cgi->m_other_character)) {
                    // it moved away, timeout trigger:
                    if (!cgi->m_other_moved_away) {
                        cgi->input(new SpeechAct(A4_TALK_PERFORMATIVE_TIMEOUT,0,0));
                        cgi->m_other_moved_away = true;
                    }
                    if (!cgi->cycle(m_character, game)) {
                        toDelete.push_back(cgi);
                    }
                } else {
                    // it died, just quit the conversation:
                    toDelete.push_back(cgi);
                }
            } else {
                // if the other character is unfriendly, or if the conversation is over, remove the instance:
                if (isUnfriendly(cgi->m_other_character->getID()) ||
                    !cgi->cycle(m_character, game)) {
                    toDelete.push_back(cgi);
                }
            }
        }
        for(ConversationGraphInstance *cgi:toDelete) {
            m_conversations.remove(cgi);
            delete cgi;
        }
    }
    
    
    // 6: behavior:
    if (m_character->isIdle()) {
        A4CharacterCommand *best = 0;
        for(A4Behavior *b:m_behaviors) {
            if (best!=0 && best->m_priority > b->getPriority()) continue;
            A4CharacterCommand *c = b->execute(m_character, game);
            if (c!=0) {
                if (best==0 || c->m_priority>best->m_priority) {
                    if (best!=0) delete best;
                    best = c;
                } else {
                    delete c;
                }
            }
        }
        // pending talks:
        if ((best==0 || best->m_priority==0) && m_character->getTalkingState()==A4CHARACTER_STATE_IDLE) {
            A4CharacterCommand *c2  = processPendingTalk(1, game);
            if (c2!=0) {
                if (best!=0) delete best;
                best = c2;
            }
        }

        // pathfinding:
        A4CharacterCommand *c = navigationCycle(game);
        if (c!=0) {
            if (best==0 || c->m_priority>best->m_priority) {
                if (best!=0) delete best;
                best = c;
            } else {
                delete c;
            }
        }

        if (best!=0) {
            m_character->issueCommand(best, game);
            delete best;
        }
    }
    clearPFTargets();

    m_cycle++;
}


WME *A4AI::updateObjectPerceptionWME(A4Object *o, int activation, bool includePosition, bool includeMap)
{
    int n_parameters = 6;
    int hash = s_object_symbol->hash_function();
    for(WME *wme2:m_memory->m_short_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == o->getID() && wme2->getNParameters()==n_parameters) {
            if (includePosition) {
                wme2->setParameter(1, WMEParameter(o->getX()), WME_PARAMETER_INTEGER);
                wme2->setParameter(2, WMEParameter(o->getY()), WME_PARAMETER_INTEGER);
                wme2->setParameter(3, WMEParameter(o->getX() + o->getPixelWidth()), WME_PARAMETER_INTEGER);
                wme2->setParameter(4, WMEParameter(o->getY() + o->getPixelHeight()), WME_PARAMETER_INTEGER);
            } else {
                wme2->setParameter(1, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(2, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(3, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(4, WMEParameter(0), WME_PARAMETER_WILDCARD);
            }
            if (includeMap) {
                if (wme2->getParameterType(5)!=WME_PARAMETER_SYMBOL ||
                    wme2->getParameter(5).m_symbol==0 ||
                    !wme2->getParameter(5).m_symbol->cmp(o->getMap()->getNameSymbol())) {
                    if (wme2->getParameter(5).m_symbol!=0 &&
                        wme2->getParameterType(5)==WME_PARAMETER_SYMBOL) delete wme2->getParameter(5).m_symbol;
                    wme2->setParameter(5, WMEParameter(new Symbol(o->getMap()->getNameSymbol())));
                }
            } else {
                if (wme2->getParameter(5).m_symbol!=0 &&
                    wme2->getParameterType(5)==WME_PARAMETER_SYMBOL) delete wme2->getParameter(5).m_symbol;
                wme2->setParameter(5, {0}, WME_PARAMETER_WILDCARD);
            }
            if (activation > wme2->getActivation()) wme2->setActivation(activation);
            updateObjectSortPerceptionWME(o, activation);
            addEmotionWME(o,activation);
            return wme2;
        }
    }
    
    // NOTE: this is a bit weird (updating long-term WMEs), think of a better way when I have time:
    for(WME *wme2:m_memory->m_long_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == o->getID() && wme2->getNParameters()==n_parameters) {
            if (includePosition) {
                wme2->setParameter(1, WMEParameter(o->getX()), WME_PARAMETER_INTEGER);
                wme2->setParameter(2, WMEParameter(o->getY()), WME_PARAMETER_INTEGER);
                wme2->setParameter(3, WMEParameter(o->getX() + o->getPixelWidth()), WME_PARAMETER_INTEGER);
                wme2->setParameter(4, WMEParameter(o->getY() + o->getPixelHeight()), WME_PARAMETER_INTEGER);
            } else {
                wme2->setParameter(1, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(2, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(3, WMEParameter(0), WME_PARAMETER_WILDCARD);
                wme2->setParameter(4, WMEParameter(0), WME_PARAMETER_WILDCARD);
            }
            if (includeMap) {
                if (wme2->getParameterType(5)!=WME_PARAMETER_SYMBOL ||
                    wme2->getParameter(5).m_symbol==0 ||
                    !wme2->getParameter(5).m_symbol->cmp(o->getMap()->getNameSymbol())) {
                    if (wme2->getParameter(5).m_symbol!=0 &&
                        wme2->getParameterType(5)==WME_PARAMETER_SYMBOL) delete wme2->getParameter(5).m_symbol;
                    wme2->setParameter(5, WMEParameter(new Symbol(o->getMap()->getNameSymbol())));
                }
            } else {
                if (wme2->getParameter(5).m_symbol!=0 &&
                    wme2->getParameterType(5)==WME_PARAMETER_SYMBOL) delete wme2->getParameter(5).m_symbol;
                wme2->setParameter(5, {0}, WME_PARAMETER_WILDCARD);
            }
            updateObjectSortPerceptionWME(o, activation);
            addEmotionWME(o,activation);
            return wme2;
        }
    }

    // add new WME:
    WME *wme;
    if (includePosition) {
        wme = new WME(s_object_symbol,
                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getX()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getY()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getX() + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getY() + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                       WMEParameter((includeMap ? new Symbol(o->getMap()->getNameSymbol()) : 0)),
                                    (includeMap ? WME_PARAMETER_SYMBOL : WME_PARAMETER_WILDCARD),
                       activation);
    } else {
        wme = new WME(s_object_symbol,
                      WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                      WMEParameter(0), WME_PARAMETER_WILDCARD,
                      WMEParameter(0), WME_PARAMETER_WILDCARD,
                      WMEParameter(0), WME_PARAMETER_WILDCARD,
                      WMEParameter(0), WME_PARAMETER_WILDCARD,
                      WMEParameter((includeMap ? new Symbol(o->getMap()->getNameSymbol()) : 0)),
                                   (includeMap ? WME_PARAMETER_SYMBOL : WME_PARAMETER_WILDCARD),
                      activation);
    }
    WME *wme2 = new WME(AIMemory::s_is_a_symbol,
                        WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                        WMEParameter(o->getSort()), WME_PARAMETER_SORT,
                        activation);
    wme->setSource(o);
    wme2->setSource(o);
    wme = addShortTermWME(wme);
    addShortTermWME(wme2);
    addEmotionWME(o,activation);
    
    return wme;
}


WME *A4AI::addEmotionWME(A4Object *o, int activation)
{
    if (o!=m_character && o->isAICharacter()) {
        int emotion = ((A4AICharacter *)o)->getEmotion();
        if (emotion != A4_EMOTION_DEFAULT) {
            WME *wme = new WME(A4Game::emotionNames[emotion],
                                WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                activation);
            wme->setSource(o);
            addShortTermWME(wme);
            return wme;
        }
    }
    return 0;
}


WME *A4AI::updateObjectSortPerceptionWME(A4Object *o, int activation)
{
    int hash = AIMemory::s_is_a_symbol->hash_function();
    for(WME *wme2:m_memory->m_short_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == o->getID()) {
            wme2->setParameter(1, WMEParameter(o->getSort()));
            if (activation > wme2->getActivation()) wme2->setActivation(activation);
            return wme2;
        }
    }
    
    // NOTE: this is a bit weird (updating long-term WMEs), think of a better way when I have time:
    for(WME *wme2:m_memory->m_long_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == o->getID()) {
            wme2->setParameter(1, WMEParameter(o->getSort()));
            if (activation > wme2->getActivation()) wme2->setActivation(activation);
            return wme2;
        }
    }
    
    // add new WME:
    WME *wme2 = new WME(AIMemory::s_is_a_symbol,
                        WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                        WMEParameter(o->getSort()), WME_PARAMETER_SORT,
                        activation);
    wme2->setSource(o);
    wme2 = addShortTermWME(wme2);
    return wme2;
}


void A4AI::updateAllObjectsCache()
{
    A4Map *map = m_character->getMap();
    int tx = m_character->getX()/tileWidth;
    int ty = m_character->getY()/tileHeight;
    int perception_x0 = m_character->getX()-tileWidth*m_sightRadius;
    int perception_y0 = m_character->getY()-tileHeight*m_sightRadius;
    int perception_x1 = m_character->getX()+m_character->getPixelWidth()+tileWidth*m_sightRadius;
    int perception_y1 = m_character->getY()+m_character->getPixelHeight()+tileHeight*m_sightRadius;
    
//    m_n_all_objects_buffer = map->getAllObjects(perception_x0, perception_y0,
//                                                perception_x1-perception_x0, perception_y1-perception_y0, m_all_objects_buffer, OBJECT_PERCEPTION_BUFFER_SIZE);
    int region = map->visiblilityRegion(tx,ty);
    m_n_all_objects_buffer = map->getAllObjectsInRegion(perception_x0, perception_y0,
                                                        perception_x1-perception_x0, perception_y1-perception_y0,
                                                        region,
                                                        m_all_objects_buffer, OBJECT_PERCEPTION_BUFFER_SIZE);
}



void A4AI::perception(A4Game *game)
{
    m_last_perception_cycle = m_cycle;
    m_sightRadius = m_character->getSightRadius();
    A4Map *map = m_character->getMap();
    tileWidth = map->getTileWidth();
    tileHeight = map->getTileHeight();
    
    int tileWidth = map->getTileWidth();
    int tileHeight = map->getTileHeight();
    int perception_x0 = m_character->getX()-tileWidth*m_sightRadius;
    int perception_y0 = m_character->getY()-tileHeight*m_sightRadius;
    int perception_x1 = m_character->getX()+m_character->getPixelWidth()+tileWidth*m_sightRadius;
    int perception_y1 = m_character->getY()+m_character->getPixelHeight()+tileHeight*m_sightRadius;
    updateAllObjectsCache();
    Sort *triggerSort = game->getOntology()->getSort("Trigger");
    for(int i = 0;i<m_n_all_objects_buffer;i++) {
        A4Object *o = m_all_objects_buffer[i];
        if (o!=m_character && !o->getBurrowed() && !o->is_a(triggerSort)) {
            updateObjectPerceptionWME(o, 100, true, true);
        }
    }
    for(A4Object *o:*(m_character->getInventory())) {
        updateObjectPerceptionWME(o, m_period*2, false, false);
    }
    {
        std::list<PerceptionBufferRecord *> *l = map->getPerceptionBuffer();
        for(PerceptionBufferRecord *pbr:*l) {
            if (pbr->m_x0<perception_x1 && pbr->m_x1>perception_x0 &&
                pbr->m_y0<perception_y1 && pbr->m_y1>perception_y0) {
                // perceived an action!:
                addPerceptionBufferWMEs(pbr);
            }
        }
        std::list<PerceptionBufferObjectWarpedRecord *> *l2 = map->getWarpPerceptionBuffer();
        for(PerceptionBufferObjectWarpedRecord *pbr:*l2) {
            if (pbr->m_x0<perception_x1 && pbr->m_x1>perception_x0 &&
                pbr->m_y0<perception_y1 && pbr->m_y1>perception_y0) {
                // perceived an object warping!:
                addPerceptionBufferWMEs(pbr);
            }
        }
        std::vector<A4MapBridge *> *l3 = map->getBridges();
        for(A4MapBridge *b:*l3) {
            if (b->getX()<perception_x1 && b->getX()+b->m_dx>perception_x0 &&
                b->getY()<perception_y1 && b->getY()+b->m_dy>perception_y0) {
                // perceived a bridge:
                WME *wme = new WME(s_bridge_symbol,
                                   WMEParameter(new Symbol(b->m_linkedTo->getMap()->getNameSymbol())), WME_PARAMETER_SYMBOL,
                                   WMEParameter(b->getX()), WME_PARAMETER_INTEGER,
                                   WMEParameter(b->getY()), WME_PARAMETER_INTEGER,
                                   WMEParameter(b->getX() + b->m_dx), WME_PARAMETER_INTEGER,
                                   WMEParameter(b->getY() + b->m_dy), WME_PARAMETER_INTEGER,
                                   WMEParameter(new Symbol(map->getNameSymbol())), WME_PARAMETER_SYMBOL,
                                   m_period*2);
                wme->setSource(b);
                addShortTermWME(wme);
            }
        }
    }
    // 2) fact checking:
    // pick a wme at random from the long term memory, and see if it contradicts perceptions:
    WME *wme = m_memory->getRandomLongTermWME();
    if (wme!=0 && wme->getFunctor() == s_object_symbol) factCheckObject(wme, map, perception_x0, perception_y0, perception_x1, perception_y1);
    if (wme!=0 && wme->getFunctor() == s_inventory_symbol) factCheckInventory(wme);
}




void A4AI::factCheckObject(WME *wme, A4Map *map,
                           int perception_x0, int perception_y0, int perception_x1, int perception_y1)
{
    if (wme->getNParameters()==6 &&
        wme->getParameterType(5) == WME_PARAMETER_SYMBOL &&
        wme->getParameter(5).m_symbol->cmp(map->getNameSymbol()) &&
        wme->getParameterType(1)==WME_PARAMETER_INTEGER) {
        int ltox0 = wme->getParameter(1).m_integer;
        int ltoy0 = wme->getParameter(2).m_integer;
        int ltox1 = wme->getParameter(3).m_integer;
        int ltoy1 = wme->getParameter(4).m_integer;
        if (ltox0<perception_x1 && ltox1>perception_x0 &&
            ltoy0<perception_y1 && ltoy1>perception_y0) {
            bool found = false;
            for(int i = 0;i<m_n_all_objects_buffer;i++) {
                A4Object *o = m_all_objects_buffer[i];
                if (!o->getBurrowed()) {
                    int otx = o->getX();
                    int oty = o->getY();
                    if (otx==ltox0 && oty==ltoy0 && wme->getParameter(0).m_integer == o->getID()) {
                        // found!
                        found = true;
                    }
                }
            }
            if (!found) {
                // perception contradicts memory!
                m_memory->preceptionContradicts(wme);
            }
        }
    }
}


void A4AI::factCheckInventory(WME *wme)
{
    bool found = false;
    for(A4Object *o:*(m_character->getInventory())) {
        if (o->getID() == wme->getParameter(0).m_integer) found = true;
    }
    if (!found) {
        // perception contradicts memory!
        m_memory->preceptionContradicts(wme);
    }
}


WME *A4AI::addPerceptionBufferWMEs(class PerceptionBufferRecord *pbr)
{
    if (pbr->m_direct_object_symbol==0 && pbr->m_direct_object_sort==0) {
        // 1 parameter:
        WME *wme = new WME(pbr->m_action, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER, PERCEIVED_ACTION_ACTIVATION);
        WME *wme2 = new WME(AIMemory::s_is_a_symbol, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER,
                                           WMEParameter(pbr->m_subject_sort), WME_PARAMETER_SORT, PERCEIVED_ACTION_ACTIVATION);
        addShortTermWME(wme);
        addShortTermWME(wme2);
        return wme;
    } else {
        // 2 parameter:
        WMEParameter p2;
        int p2_type;
        if (pbr->m_direct_object_symbol!=0) {
            p2.m_symbol = new Symbol(pbr->m_direct_object_symbol);
            p2_type = WME_PARAMETER_SYMBOL;
        } else {
            if (pbr->m_direct_object_ID!=0) {
                p2.m_integer = pbr->m_direct_object_ID;
                p2_type = WME_PARAMETER_INTEGER;
                WME *wme3 = new WME(AIMemory::s_is_a_symbol, WMEParameter(pbr->m_direct_object_ID), WME_PARAMETER_INTEGER,
                                                   WMEParameter(pbr->m_direct_object_sort), WME_PARAMETER_SORT, PERCEIVED_ACTION_ACTIVATION);
                addShortTermWME(wme3);
            } else {
                p2.m_sort = pbr->m_direct_object_sort;
                p2_type = WME_PARAMETER_SORT;
            }
        }
        if (pbr->m_indirect_object_sort==0) {
            // 3 parameter:
            WME *wme = new WME(pbr->m_action, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER,
                                              p2, p2_type, PERCEIVED_ACTION_ACTIVATION);
            WME *wme2 = new WME(AIMemory::s_is_a_symbol, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER,
                                               WMEParameter(pbr->m_subject_sort), WME_PARAMETER_SORT, PERCEIVED_ACTION_ACTIVATION);
            addShortTermWME(wme);
            addShortTermWME(wme2);
            return wme;
        } else {
            WME *wme = new WME(pbr->m_action, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER,
                                              p2, p2_type,
                                              WMEParameter(pbr->m_indirect_object_ID), WME_PARAMETER_INTEGER,
                                              PERCEIVED_ACTION_ACTIVATION);
            WME *wme2 = new WME(AIMemory::s_is_a_symbol, WMEParameter(pbr->m_subject_ID), WME_PARAMETER_INTEGER,
                                               WMEParameter(pbr->m_subject_sort), WME_PARAMETER_SORT, PERCEIVED_ACTION_ACTIVATION);
            WME *wme4 = new WME(AIMemory::s_is_a_symbol, WMEParameter(pbr->m_indirect_object_ID), WME_PARAMETER_INTEGER,
                                               WMEParameter(pbr->m_indirect_object_sort), WME_PARAMETER_SORT, PERCEIVED_ACTION_ACTIVATION);
            addShortTermWME(wme);
            addShortTermWME(wme2);
            addShortTermWME(wme4);
            return wme;
        }
    }
}


WME *A4AI::addPerceptionBufferWMEs(PerceptionBufferObjectWarpedRecord *pbr)
{
    int n_parameters = 6;
    int hash = s_object_symbol->hash_function();
    for(WME *wme2:m_memory->m_short_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(s_object_symbol) &&
            wme2->getParameter(0).m_integer == pbr->m_ID && wme2->getNParameters()==n_parameters) {
            wme2->setParameter(1, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(2, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(3, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(4, {0}, WME_PARAMETER_WILDCARD);
            if (wme2->getParameter(5).m_symbol==0 ||
                !wme2->getParameter(5).m_symbol->cmp(pbr->m_target_map)) {
                if (wme2->getParameter(5).m_symbol!=0) delete wme2->getParameter(5).m_symbol;
                wme2->setParameter(5, WMEParameter(new Symbol(pbr->m_target_map)), WME_PARAMETER_SYMBOL);
            }
            if (PERCEIVED_WARP_ACTIVATION > wme2->getActivation()) wme2->setActivation(PERCEIVED_WARP_ACTIVATION);
            addPerceptionBufferSortWMEs(pbr);
            return wme2;
        }
    }
    
    // NOTE: this is a bit weird (updating long-term WMEs), think of a better way when I have time:
    for(WME *wme2:m_memory->m_long_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(s_object_symbol) &&
            wme2->getParameter(0).m_integer == pbr->m_ID && wme2->getNParameters()==n_parameters) {
            wme2->setParameter(1, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(2, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(3, {0}, WME_PARAMETER_WILDCARD);
            wme2->setParameter(4, {0}, WME_PARAMETER_WILDCARD);
            if (wme2->getParameter(5).m_symbol==0 ||
                !wme2->getParameter(5).m_symbol->cmp(pbr->m_target_map)) {
                if (wme2->getParameter(5).m_symbol!=0) delete wme2->getParameter(5).m_symbol;
                wme2->setParameter(5, WMEParameter(new Symbol(pbr->m_target_map)), WME_PARAMETER_SYMBOL);
            }
            if (PERCEIVED_WARP_ACTIVATION > wme2->getActivation()) wme2->setActivation(PERCEIVED_WARP_ACTIVATION);
            addPerceptionBufferSortWMEs(pbr);
            return wme2;
        }
    }
    
    // add new WME:
    WME *wme = new WME(s_object_symbol,
                       WMEParameter(pbr->m_ID), WME_PARAMETER_INTEGER,
                       WMEParameter(0), WME_PARAMETER_WILDCARD,
                       WMEParameter(0), WME_PARAMETER_WILDCARD,
                       WMEParameter(0), WME_PARAMETER_WILDCARD,
                       WMEParameter(0), WME_PARAMETER_WILDCARD,
                       WMEParameter(new Symbol(pbr->m_target_map)), WME_PARAMETER_SYMBOL,
                       PERCEIVED_WARP_ACTIVATION);
    WME *wme2 = new WME(AIMemory::s_is_a_symbol,
                        WMEParameter(pbr->m_ID), WME_PARAMETER_INTEGER,
                        WMEParameter(pbr->m_sort), WME_PARAMETER_SORT,
                        PERCEIVED_WARP_ACTIVATION);
    wme = addShortTermWME(wme);
    addShortTermWME(wme2);
    return wme;
}


WME *A4AI::addPerceptionBufferSortWMEs(PerceptionBufferObjectWarpedRecord *pbr)
{
    int hash = AIMemory::s_is_a_symbol->hash_function();
    for(WME *wme2:m_memory->m_short_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == pbr->m_ID) {
            wme2->setParameter(1, WMEParameter(pbr->m_sort));
            if (PERCEIVED_WARP_ACTIVATION > wme2->getActivation()) wme2->setActivation(PERCEIVED_WARP_ACTIVATION);
            return wme2;
        }
    }
    
    // NOTE: this is a bit weird (updating long-term WMEs), think of a better way when I have time:
    for(WME *wme2:m_memory->m_long_term_memory[hash]) {
        if (wme2->getParameter(0).m_integer == pbr->m_ID) {
            wme2->setParameter(1, WMEParameter(pbr->m_sort));
            if (PERCEIVED_WARP_ACTIVATION > wme2->getActivation()) wme2->setActivation(PERCEIVED_WARP_ACTIVATION);
            return wme2;
        }
    }
    
    // add new WME:
    WME *wme2 = new WME(AIMemory::s_is_a_symbol,
                        WMEParameter(pbr->m_ID), WME_PARAMETER_INTEGER,
                        WMEParameter(pbr->m_sort), WME_PARAMETER_SORT,
                        PERCEIVED_WARP_ACTIVATION);
    wme2 = addShortTermWME(wme2);
    return wme2;
}


void A4AI::receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa) {
    // if the character cannot talk, just ignore
    if (m_conversationGraph==0) return;
    
    sa = new SpeechAct(sa);
    if (speaker==m_character) {
        // self is talking:
        for(ConversationGraphInstance *cgi:m_conversations) {
            if (cgi->m_other_character == receiver) {
                cgi->inputFromSelf(sa);
                return;
            }
        }
        ConversationGraphInstance *cgi = new ConversationGraphInstance(m_conversationGraph, receiver);
        m_conversations.push_back(cgi);
        cgi->inputFromSelf(sa);
    } else {
        // the other is talking:
        for(ConversationGraphInstance *cgi:m_conversations) {
            if (cgi->m_other_character == speaker) {
                cgi->input(sa);
                return;
            }
        }
        ConversationGraphInstance *cgi = new ConversationGraphInstance(m_conversationGraph, speaker);
        m_conversations.push_back(cgi);
        cgi->input(sa);
    }
}



bool A4AI::willAcceptSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa)
{
    if (m_conversationGraph==0) return false;
    if (speaker==m_character) {
        // self is talking:
        ConversationGraphInstance *cgi = 0;
        for(ConversationGraphInstance *cgi2:m_conversations) {
            if (cgi2->m_other_character == receiver) {
                cgi = cgi2;
                break;
            }
        }
        if (cgi==0) {
            cgi = new ConversationGraphInstance(m_conversationGraph, receiver);
            m_conversations.push_back(cgi);
        }
        
        return cgi->willAcceptSpeechActFromSelf(sa);
    } else {
        // the other is talking:
        ConversationGraphInstance *cgi = 0;
        for(ConversationGraphInstance *cgi2:m_conversations) {
            if (cgi2->m_other_character == speaker) {
                cgi = cgi2;
                break;
            }
        }
        if (cgi==0) {
            cgi = new ConversationGraphInstance(m_conversationGraph, speaker);
            m_conversations.push_back(cgi);
        }
        
        return cgi->willAcceptSpeechAct(sa);
    }
}


A4CharacterCommand *A4AI::processPendingTalk(int priority, A4Game *game)
{
    for(A4Script *s:m_pendingTalk) {
        Sort *sort = game->getOntology()->getSort(s->ID);
        for(int i = 0;i<m_n_all_objects_buffer;i++) {
            A4Object *o = m_all_objects_buffer[i];
            if (o->getSort()->is_a(sort) &&
                !isUnfriendly(o->getSort())) {
                // match found, try to talk!
                ConversationGraphInstance *cgi = 0;
                for(ConversationGraphInstance *cgi2:m_conversations) {
                    if (cgi2->m_other_character == o) {
                        cgi = cgi2;
                        break;
                    }
                }
                if (cgi==0 || cgi->m_currentState->m_name->cmp("none")) {
                    // we are not talking to the other character yet, start a conversation:
                    char *buffer = new char[3]; strcpy(buffer, "hi");
                    char *buffer2 = new char[10]; strcpy(buffer2, "Good day!");
                    SpeechAct *sa = new SpeechAct(A4_TALK_PERFORMATIVE_HI, buffer, buffer2);
                    return new A4CharacterCommand(A4CHARACTER_COMMAND_TALK, 0, 0, o, sa, priority);
                } else {
                    // we are already talking to the other character:
                    char *text = new char[strlen(s->text)+1];
                    strcpy(text, s->text);
                    SpeechAct *sa = new SpeechAct(A4_TALK_PERFORMATIVE_INFORM, 0, text);
                    if ((cgi->m_script_queue==0 || cgi->m_script_queue->scripts.size()==0) &&
                        cgi->m_other_queue.size()==0 &&
                        cgi->m_self_queue.size()==0 &&
                        cgi->willAcceptSpeechActFromSelf(sa)) {
                        // we can talk!
                        // execute the subscripts:
                        A4ScriptExecutionQueue *sq = new A4ScriptExecutionQueue(m_character,m_character->getMap(),game,(A4Character *)o);
                        char *text2 = new char[strlen(s->text)+1];
                        strcpy(text2, s->text);
                        A4Script *talk_script = new A4Script(A4_SCRIPT_TALK, 0, text2, A4_TALK_PERFORMATIVE_INFORM, false, true);
                        sq->scripts.push_back(talk_script);
                        for(A4Script *s2:s->subScripts) sq->scripts.push_back(new A4Script(s2));
                        m_character->addScriptQueue(sq);
                        delete s;
                        m_pendingTalk.remove(s);
                        delete sa;
                        return 0;
//                        return new A4CharacterCommand(A4CHARACTER_COMMAND_TALK, 0, 0, o, sa, priority);
                    } else {
                        // we cannot talk yet (wait):
                        delete sa;
                    }
                }
            }
        }
    }
    return 0;
}


void A4AI::updateNavigationPerceptionBuffer(bool force)
{
    A4Object *subject = m_character;
    if (m_character->isInVehicle()) subject = m_character->getVehicle();
    
    if (!force && m_navigationBuffer!=0 && m_navigationBuffer_lastUpdated > m_cycle-m_period) return;

    if (m_navigationBuffer == 0) {
        m_navigationBuffer_size = m_sightRadius*2 + subject->getPixelWidth()/subject->getMap()->getTileWidth();
        m_navigationBuffer = new int[m_navigationBuffer_size*m_navigationBuffer_size];
        m_navigationBuffer_bridges = new A4MapBridge *[m_navigationBuffer_size*m_navigationBuffer_size];
    }

    A4Map *map =subject->getMap();
    m_navigationBuffer_map = map;
    m_navigationBuffer_mapDx = map->getDx();
    int cx = (subject->getX() + subject->getPixelWidth()/2) / tileWidth;
    int cy = (subject->getY() + subject->getPixelHeight()/2) / tileHeight;
    m_navigationBuffer_x = cx - (m_sightRadius + (subject->getPixelWidth()/2)/tileWidth);
    m_navigationBuffer_y = cy - (m_sightRadius + (subject->getPixelHeight()/2)/tileHeight);

    for(int i = 0;i<m_navigationBuffer_size;i++) {
        for(int j = 0;j<m_navigationBuffer_size;j++) {
            m_navigationBuffer_bridges[j+i*m_navigationBuffer_size] = 0;
            if (map->walkableOnlyBackground((m_navigationBuffer_x+j)*tileWidth,(m_navigationBuffer_y+i)*tileHeight, tileWidth, tileHeight, subject)) {
                m_navigationBuffer[j+i*m_navigationBuffer_size] = NAVIGATION_BUFFER_WALKABLE;
            } else {
                m_navigationBuffer[j+i*m_navigationBuffer_size] = NAVIGATION_BUFFER_NOT_WALKABLE;
            }
        }
    }
    
    // add objects:
    for(int i = 0;i<m_n_all_objects_buffer;i++) {
        A4Object *o = m_all_objects_buffer[i];
        if (o!=m_character && !o->isWalkable() && o!=m_character->getVehicle()) {
            int x0 = (o->getX()/tileWidth) - m_navigationBuffer_x;
            int y0 = (o->getY()/tileHeight) - m_navigationBuffer_y;
            int x1 = ((o->getX()+o->getPixelWidth()-1)/tileWidth) - m_navigationBuffer_x;
            int y1 = ((o->getY()+o->getPixelHeight()-1)/tileHeight) - m_navigationBuffer_y;
            for(int y = y0;y<=y1;y++) {
                if (y>=0 && y<m_navigationBuffer_size) {
                    for(int x = x0;x<=x1;x++) {
                        if (x>=0 && x<m_navigationBuffer_size) {
                            m_navigationBuffer[x+y*m_navigationBuffer_size] = NAVIGATION_BUFFER_NOT_WALKABLE;
                        }
                    }
                }
            }
        }
    }
    
    // add bridges:
    for(A4MapBridge *b:*map->getBridges()) {
        int x0 = (b->getX()/tileWidth) - m_navigationBuffer_x;
        int y0 = (b->getY()/tileHeight) - m_navigationBuffer_y;
        int x1 = ((b->getX()+b->m_dx-1)/tileWidth) - m_navigationBuffer_x;
        int y1 = ((b->getY()+b->m_dy-1)/tileHeight) - m_navigationBuffer_y;
        for(int y = y0;y<=y1;y++) {
            if (y>=0 && y<m_navigationBuffer_size) {
                for(int x = x0;x<=x1;x++) {
                    if (x>=0 && x<m_navigationBuffer_size) {
                        if (m_navigationBuffer[x+y*m_navigationBuffer_size] == NAVIGATION_BUFFER_WALKABLE) {
                            m_navigationBuffer[x+y*m_navigationBuffer_size] = NAVIGATION_BUFFER_BRIDGE;
                            m_navigationBuffer_bridges[x+y*m_navigationBuffer_size] = b;
                        }
                    }
                }
            }
        }
    }
    
    m_navigationBuffer_lastUpdated = m_cycle;
}



void A4AI::clearPFTargets()
{
    m_pathfinding_n_targets_old = m_pathfinding_n_targets;
    m_pathfinding_n_targets = 0;
}


void A4AI::addPFTarget(int tilex0, int tiley0, int tilex1, int tiley1, int action, int priority, bool flee, A4Object *target)
{
    for(int i = 0;i<m_pathfinding_n_targets;i++) {
        if (m_pathfinding_targets[i].m_flee) continue;
        if (m_pathfinding_targets[i].m_x0 == tilex0 && m_pathfinding_targets[i].m_y0 == tiley0 &&
            m_pathfinding_targets[i].m_x1 == tilex1 && m_pathfinding_targets[i].m_y1 == tiley1) {
            if (m_pathfinding_targets[i].m_priority<=priority) {
                m_pathfinding_targets[i].m_action = action;
                m_pathfinding_targets[i].m_priority = priority;
                m_pathfinding_targets[i].m_target = target;
            }
            return;
        }
    }
    if (m_pathfinding_n_targets<MAX_PATHFINDING_TARGETS) {
        m_pathfinding_targets[m_pathfinding_n_targets].m_x0 = tilex0;
        m_pathfinding_targets[m_pathfinding_n_targets].m_y0 = tiley0;
        m_pathfinding_targets[m_pathfinding_n_targets].m_x1 = tilex1;
        m_pathfinding_targets[m_pathfinding_n_targets].m_y1 = tiley1;
        m_pathfinding_targets[m_pathfinding_n_targets].m_action = action;
        m_pathfinding_targets[m_pathfinding_n_targets].m_priority = priority;
        m_pathfinding_targets[m_pathfinding_n_targets].m_flee = flee;
        m_pathfinding_targets[m_pathfinding_n_targets].m_target = target;
        m_pathfinding_n_targets++;
    }
}


void A4AI::addPFTargetWME(WME *w, A4Game *a_game, int action, int priority, bool flee)
{
    if (w->getParameterType(5)==WME_PARAMETER_SYMBOL) {
        if (m_navigationBuffer_lastUpdated == -1 ||
            m_navigationBuffer_lastUpdated <= m_cycle-m_period) {
            updateNavigationPerceptionBuffer(false);
        }
        A4Object *target = w->getSource();
        if (target!=0 && !m_navigationBuffer_map->contains(target)) target = 0;
                
        A4Map *map2 = a_game->getMap(w->getParameter(5).m_symbol);
        if (map2==m_navigationBuffer_map) {
            // same map where a_character is:
            if (w->getParameterType(1)==WME_PARAMETER_INTEGER) {
                addPFTarget(w->getParameter(1).m_integer,
                            w->getParameter(2).m_integer,
                            w->getParameter(3).m_integer,
                            w->getParameter(4).m_integer,
                            action, priority, flee, target);
            } else {
                // we are the the right map, but we don't know where the object is, so nothing to be done ...
            }
        } else {
            // different map, just target the map:
            std::vector<WME *> *l = m_memory->retrieveByFunctor(s_bridge_symbol);
            for(WME *wme:*l) {
                if (wme->getParameterType(0) == WME_PARAMETER_SYMBOL &&
                    wme->getParameterType(1) == WME_PARAMETER_INTEGER &&
                    wme->getParameterType(5) == WME_PARAMETER_SYMBOL &&
                    wme->getParameter(5).m_symbol->cmp(m_navigationBuffer_map->getNameSymbol()) &&
                    wme->getParameter(0).m_symbol->cmp(w->getParameter(5).m_symbol)) {
                    addPFTarget(wme->getParameter(1).m_integer,
                                wme->getParameter(2).m_integer,
                                wme->getParameter(3).m_integer,
                                wme->getParameter(4).m_integer,
                                A4CHARACTER_COMMAND_IDLE, priority, flee, 0);
                }
            }
            delete l;
        }
    } else {
        // we don't know where the obejct is, so nothing to be done ...
    }
}


bool A4AI::isFriendly(int ID)
{
    if (m_is_a_pattern==0) {
        m_is_a_pattern = new WME(AIMemory::s_is_a_symbol,
                                 WMEParameter(ID), WME_PARAMETER_INTEGER,
                                 WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
    } else {
        m_is_a_pattern->setParameter(0, WMEParameter(ID));
    }
    WME *w = m_memory->retrieveFirstBySubsumption(m_is_a_pattern);
    if (w==0) return false;
    return isFriendly(w->getParameter(1).m_sort);
}


bool A4AI::isFriendly(Sort *s)
{
    if (m_is_friendly_sort_pattern==0) {
        m_is_friendly_sort_pattern = new WME(A4AI::s_friendly_symbol,
                                             WMEParameter(s), WME_PARAMETER_SORT, 0);
    } else {
        m_is_friendly_sort_pattern->setParameter(0, WMEParameter(s));
    }
    bool tmp = m_memory->isSubsumedByAnyWME(m_is_friendly_sort_pattern);
    return tmp;
}


bool A4AI::isUnfriendly(int ID)
{
    if (m_is_a_pattern==0) {
        m_is_a_pattern = new WME(AIMemory::s_is_a_symbol,
                                          WMEParameter(ID), WME_PARAMETER_INTEGER,
                                          WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
    } else {
        m_is_a_pattern->setParameter(0, WMEParameter(ID));
    }
    WME *w = m_memory->retrieveFirstBySubsumption(m_is_a_pattern);
    if (w==0) return false;
    return isUnfriendly(w->getParameter(1).m_sort);
}


bool A4AI::isUnfriendly(Sort *s)
{
    if (m_is_unfriendly_sort_pattern==0) {
        m_is_unfriendly_sort_pattern = new WME(A4AI::s_unfriendly_symbol,
                                               WMEParameter(s), WME_PARAMETER_SORT, 0);
    } else {
        m_is_unfriendly_sort_pattern->setParameter(0, WMEParameter(s));
    }
    bool tmp = m_memory->isSubsumedByAnyWME(m_is_unfriendly_sort_pattern);
    return tmp;
}


void A4AI::saveToXML(class XMLwriter *w, A4Game *game)
{
    w->openTag("AI");

    w->openTag("attribute");
    w->setAttribute("name", "period");
    w->setAttribute("value", m_period);
    w->closeTag("attribute");
    w->openTag("attribute");
    w->setAttribute("name", "cycle");
    w->setAttribute("value", m_cycle);
    w->closeTag("attribute");
    
    /*
     AIMemory *m_memory;
     std::vector<class A4Behavior *> m_behaviors;
     std::vector<InferenceRule *> m_inferenceRules;
     
     ConversationGraph *m_conversationGraph;
     std::vector<A4Script *> m_pendingTalk;
     */
    
    w->closeTag("AI");
}


void A4AI::initSorts(Ontology *o)
{
    if (s_object_symbol==0) {
        s_object_symbol = new Symbol("object");
        s_inventory_symbol = new Symbol("inventory");
        s_location_symbol = new Symbol("location");
        s_bridge_symbol = new Symbol("bridge");
        s_action_take = new Symbol("take");
        s_action_drop = new Symbol("drop");
        s_action_die = new Symbol("die");
        s_action_attack = new Symbol("attack");
        s_action_cast = new Symbol("cast");
        s_action_interact = new Symbol("interact");
        s_action_embark = new Symbol("embark");
        s_action_disembark = new Symbol("disembark");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_HI] = new Symbol("talk:hi");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_BYE] = new Symbol("talk:bye");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_ASK] = new Symbol("talk:ask");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_INFORM] = new Symbol("talk:inform");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_TRADE] = new Symbol("talk:trade");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_END_TRADE] = new Symbol("talk:endtrade");
        s_action_talk_performatives[A4_TALK_PERFORMATIVE_TIMEOUT] = new Symbol("talk:timeout");
        s_action_give = new Symbol("give");
        s_action_buy = new Symbol("buy");
        s_friendly_symbol = new Symbol("friendly");
        s_unfriendly_symbol = new Symbol("unfriendly");
    }
    
    o->newSort(new Symbol("Object"));

    o->newSort(new Symbol("Item"), o->getSort("Object"));
    o->newSort(new Symbol("CoinPurse"), o->getSort("Item"));
    o->newSort(new Symbol("Potion"), o->getSort("Item"));
    o->newSort(new Symbol("HPPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("MPPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("XPPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("StrengthPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("ConstitutionPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("LifePotion"), o->getSort("Potion"));
    o->newSort(new Symbol("PowerPotion"), o->getSort("Potion"));
    o->newSort(new Symbol("Key"), o->getSort("Item"));
    o->newSort(new Symbol("Door"), o->getSort("Object"));
    o->newSort(new Symbol("EquipableItem"), o->getSort("Item"));
    o->newSort(new Symbol("Food"), o->getSort("Item"));
    o->newSort(new Symbol("Container"), o->getSort("Item"));
    o->newSort(new Symbol("Spade"), o->getSort("Item"));
    o->newSort(new Symbol("Scroll"), o->getSort("Item"));
    o->newSort(new Symbol("Wand"), o->getSort("Item"));

    o->newSort(new Symbol("PushableWall"), o->getSort("Object"));
    o->newSort(new Symbol("PressurePlate"), o->getSort("Object"));
    o->newSort(new Symbol("Lever"), o->getSort("Object"));
    o->newSort(new Symbol("Trigger"), o->getSort("Object"));
    o->newSort(new Symbol("Spell"), o->getSort("Object"));
    o->newSort(new Symbol("SpellMagicMissile"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellHeal"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellShield"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellIncrease"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellDecrease"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellFireball"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellMagicEye"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellRegenerate"), o->getSort("Spell"));
    o->newSort(new Symbol("SpellIncinerate"), o->getSort("Spell"));

    o->newSort(new Symbol("WalkingObject"), o->getSort("Object"));
    o->newSort(new Symbol("Character"), o->getSort("WalkingObject"));
    o->newSort(new Symbol("Vehicle"), o->getSort("WalkingObject"));
}


