//
//  ConversationGraph.h
//  A4Engine
//
//  Created by Santiago Ontanon on 5/31/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__ConversationGraph__
#define __A4Engine__ConversationGraph__

#include <list>
#include <vector>
#include "Symbol.h"
#include "XMLparser.h"
#include "XMLwriter.h"

#define CGT_ACTOR_SELF 0
#define CGT_ACTOR_OTHER 1


class ConversationGraphTransition {
public:
    ConversationGraphTransition();
    ConversationGraphTransition(XMLNode *xml);
    ~ConversationGraphTransition();
    
    void saveToXML(XMLwriter *w);
    
    bool match(int actor, SpeechAct *sa);
    bool moreSpecificThan(ConversationGraphTransition *t);
    
    int m_trigger_actor;
    int m_trigger_performative;
    Symbol *m_trigger_keyword;
    
    Symbol *m_state;
    bool m_consume;
    
    std::vector<class A4Script *> m_effects;
};


class ConversationGraphState {
public:
    ConversationGraphState(Symbol *name);
    ConversationGraphState(XMLNode *xml);
    ~ConversationGraphState();
    
    void saveToXML(XMLwriter *w);    
    
    void addConversationGraphTransition(ConversationGraphTransition *t);
    
    Symbol *m_name;
    std::list<ConversationGraphTransition *> m_transitions;
};


class ConversationGraph {
    friend class ConversationGraphInstance;
public:
    ConversationGraph();
    ConversationGraph(XMLNode *xml);
    ~ConversationGraph();
    
    void saveToXML(XMLwriter *w);
    
    ConversationGraphState *getState(const char *state);
    ConversationGraphState *getState(Symbol *state);
    
    void addConversationGraphTransition(XMLNode *xml);
    void addConversationGraphTransition(Symbol *state, ConversationGraphTransition *t);
    
private:
    std::vector<ConversationGraphState *> m_states;
};


class ConversationGraphInstance {
public:
    ConversationGraphInstance(ConversationGraph *g, A4Character *oc);
    ~ConversationGraphInstance();
    
    void input(SpeechAct *sa);
    void inputFromSelf(SpeechAct *sa);
    bool cycle(A4Character *c, A4Game *game);
    
    bool willAcceptSpeechAct(SpeechAct *sa);
    bool willAcceptSpeechActFromSelf(SpeechAct *sa);

    A4Character *m_other_character;
    ConversationGraph *m_graph;
    ConversationGraphState *m_currentState;
    int m_timer;
    std::list<class SpeechAct *> m_other_queue; // SpeechActs coming from the other character
    std::list<class SpeechAct *> m_self_queue;  // SpeechActs coming from self
    A4ScriptExecutionQueue *m_script_queue;
    bool m_other_moved_away;
};

#endif /* defined(__A4Engine__ConversationGraph__) */
