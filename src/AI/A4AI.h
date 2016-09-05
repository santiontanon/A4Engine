//
//  A4AI.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4AI__
#define __A4Engine__A4AI__

#include <list>
#include "Sort.h"
#include "Ontology.h"
#include "AIMemory.h"
#include "InferenceRule.h"

/*
 
 Everything that is A4 related about the AI should go in this class.
 Other code, such as Sort, Ontology, WME and AIMemory should remain domain-independent.
 
 */

#define NAVIGATION_BUFFER_WALKABLE      0
#define NAVIGATION_BUFFER_NOT_WALKABLE  1
#define NAVIGATION_BUFFER_BRIDGE        2

#define MAX_PATHFINDING_TARGETS     16
#define MAX_UN_FRIENDLY             32

#define OBJECT_PERCEPTION_BUFFER_SIZE   64

class PFTarget {
public:
    int m_x0, m_y0;
    int m_x1, m_y1;
    int m_priority;
    int m_action;
    bool m_flee;
    A4Object *m_target;
};


class A4AI {
public:
    A4AI(class A4AICharacter *c, int sightRadius);
    ~A4AI();
    
    void saveToXML(class XMLwriter *w, A4Game *game);    

    void cycle(A4Game *game);
    void updateAllObjectsCache();
    void perception(A4Game *game);
    class A4CharacterCommand *navigationCycle(A4Game *game);

    WME *updateObjectPerceptionWME(A4Object *o, int activation, bool includePosition, bool includeMap);
    WME *updateObjectSortPerceptionWME(A4Object *o, int activation);
    WME *addEmotionWME(A4Object *o, int activation);
    
    int getCycle() {return m_cycle;}
    void setCycle(int c) {m_cycle = c;}
    int getPeriod() {return m_period;}
    void setPeriod(int p) {m_period = p;}
    void addPendingTalk(A4Script *s) {m_pendingTalk.push_back(s);}
    std::list<A4Script *> *getPendingTalk() {return &m_pendingTalk;}

    WME *addShortTermWME(WME *wme);
    WME *addLongTermWME(WME *wme);
    WME *addPerceptionBufferWMEs(class PerceptionBufferRecord *pbr);
    WME *addPerceptionBufferWMEs(class PerceptionBufferObjectWarpedRecord *pbr);
    WME *addPerceptionBufferSortWMEs(class PerceptionBufferObjectWarpedRecord *pbr);

    AIMemory *getMemory() {return m_memory;}
    int getSightRadius() {return m_sightRadius;}
    A4Object **getObjectPerceptionCache() {return m_all_objects_buffer;};
    int getObjectPerceptionCacheSize() {return m_n_all_objects_buffer;};

    void addBehavior(class A4Behavior *b) {m_behaviors.push_back(b);}
    std::list<A4Behavior *> *getBehaviors() {return &m_behaviors;}
    void addInferenceRule(InferenceRule *r) {m_inferenceRules.push_back(r);}
    std::vector<InferenceRule *> *getInferenceRules() {return &m_inferenceRules;}

    void setConversationGraph(class ConversationGraph *cg) {m_conversationGraph = cg;}
    ConversationGraph *getConversationGraph() {return m_conversationGraph;}

    void receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa);
    bool willAcceptSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa);

    A4CharacterCommand *processPendingTalk(int priority, A4Game *game);
    
    void updateNavigationPerceptionBuffer(bool force);

    // pathfinding:
    void clearPFTargets();
    void addPFTarget(int tilex0, int tiley0, int tilex1, int tiley1, int action, int priority, bool flee, A4Object *target);
    void addPFTargetWME(WME *wme, class A4Game *a_game, int action, int priority, bool flee);

    A4CharacterCommand *pixelLevelPathFinding(A4Object *subject);
    bool pathFinding(A4Object *subject);
    void pathFindingScore(int tilex, int tiley, int &score, int &priority, A4Object *subject);

    // fact check:
    void factCheckObject(WME *wme, A4Map *map,
                         int perception_x0, int perception_y0, int perception_x1, int perception_y1);
    void factCheckInventory(WME *wme);

    // common functions used by several behaviors:
    bool isFriendly(int ID);
    bool isFriendly(Sort *s);
    bool isUnfriendly(int ID);
    bool isUnfriendly(Sort *s);

    // quick access symbols:
    static void initSorts(Ontology *o);
    static Symbol *s_object_symbol,*s_location_symbol,*s_bridge_symbol;
    static Symbol *s_inventory_symbol;
    static Symbol *s_action_take,*s_action_drop,*s_action_die,*s_action_attack,*s_action_cast,*s_action_interact,*s_action_embark,*s_action_disembark, *s_action_give, *s_action_buy;
    static Symbol *s_action_talk_performatives[A4_TALK_N_PERFORMATIVES];
    static Symbol *s_friendly_symbol, *s_unfriendly_symbol;

    // friendly/unfriendly cache:
    WME *m_friendly_cache[MAX_UN_FRIENDLY];
    WME *m_unfriendly_cache[MAX_UN_FRIENDLY];
    int m_n_friendly, m_n_unfriendly;

    // navigation perception buffer (this is public, since it will be used by all the behaviors):
    int m_navigationBuffer_lastUpdated;
    int *m_navigationBuffer;
    A4MapBridge **m_navigationBuffer_bridges;
    int m_navigationBuffer_size, m_navigationBuffer_mapDx;
    int m_navigationBuffer_x,m_navigationBuffer_y;
    A4Map *m_navigationBuffer_map;

    // pathfinding: all in tile coordinates
    int m_pathfinding_result_x, m_pathfinding_result_y, m_pathfinding_result_priority;
    int m_pathfinding_result_offset_x, m_pathfinding_result_offset_y;

    PFTarget m_pathfinding_targets[MAX_PATHFINDING_TARGETS];
    int m_pathfinding_n_targets;
    int m_pathfinding_n_targets_old;    // this is just to be used by the AI debugger
    
    int *m_pathfinding_closed, *m_pathfinding_open; // internal buffers for pathfinding
    int m_pathFinding_lastUpdated, m_pathFinding_iterations;    // for debugging purposes
    int m_nextPathFindingTimer;

protected:
    int m_period;       // the AI will only run once each m_period cycles
    int m_cycle;        // current cycle
    int m_sightRadius;
    int m_last_perception_cycle;
    class A4AICharacter *m_character;
    AIMemory *m_memory;
    std::list<class A4Behavior *> m_behaviors;
    std::vector<InferenceRule *> m_inferenceRules;
    std::list<class ConversationGraphInstance *> m_conversations;
    
    ConversationGraph *m_conversationGraph;
    std::list<A4Script *> m_pendingTalk;
    
    // some values cached for efficiency:
    int tileWidth,tileHeight;
//    std::vector<A4Object *> *m_all_objects;    // this is filled at each cycle by "perception"
    A4Object *m_all_objects_buffer[OBJECT_PERCEPTION_BUFFER_SIZE];
    int m_n_all_objects_buffer;
    
    // cached WMEs:
    WME *m_is_a_pattern;
    WME *m_is_friendly_sort_pattern;
    WME *m_is_unfriendly_sort_pattern;
    
};

#endif /* defined(__A4Engine__A4AI__) */
