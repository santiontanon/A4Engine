//
//  InferenceRule.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/30/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__InferenceRule__
#define __A4Engine__InferenceRule__

#include <list>

#include "WME.h"
#include "A4Script.h"


typedef struct {
    int m_variable;
    WMEParameter m_value;
    int m_value_type;
} Binding;


class InferenceRule {
public:
    InferenceRule(class XMLNode *xml, class Ontology *o);
    ~InferenceRule();
    
    void saveToXML(class XMLwriter *w);

    void execute(class AIMemory *m, A4Object *character, A4Map *map, A4Game *game);
    int executeScriptEffects(A4Object *p, A4Map *map, A4Game *game, A4Character *otherCharacter);
    bool matchPremise(std::list<WME *>::iterator it, std::list<WME *>::iterator end, std::list<Binding *> *bindings, AIMemory *m);
    void applyBindings(WME *wme, std::list<Binding *> *bindings);

    double getFrequency() {return m_frequency;}

protected:
    double m_frequency;
    std::list<WME *> m_premise;
    WME *m_conclusion;
    int m_activation;

    int m_coolDown;
    bool m_once;
    std::vector<class A4Script *> m_effects;
    
    int m_last_cycle_executed;
};

#endif /* defined(__A4Engine__InferenceRule__) */
