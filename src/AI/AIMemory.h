//
//  AIMemory.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__AIMemory__
#define __A4Engine__AIMemory__

#include <list>
#include <vector>
#include "WME.h"


class AIMemory {
    friend class A4AI;
    
public:
    AIMemory(int freeze_threshold);
    ~AIMemory();
    void cycle(int steps);      // how many time steps to simulate

    int getFreezeThreshold() {return m_freeze_threshold;}
    
    WME *contains(WME *wme);    // returns a WME that exactly matches "wme" if it exists, and 0 otherwise

    WME *addShortTermWME(WME *wme); // these two functions return the pointer to the WME after adding ("wme" if it did not exist, or the previous WME if it did)
    WME *addLongTermWME(WME *wme);  // (and thus was not added)
    void removeWME(WME *wme);

    std::list<WME *> *getShortTermWMEs() {return &m_short_term_memory_plain;}
    std::list<WME *> *getLongTermWMEs() {return &m_long_term_memory_plain;}

    std::vector<WME *> *retrieveByFunctor(Symbol *functor);
    std::vector<WME *> *retrieveSubsumption(WME *pattern);
    std::vector<WME *> *retrieveUnification(WME *pattern);
    std::vector<WME *> *retrieveRelativeSubsumption(WME *pattern);

    bool isSubsumedByAnyWME(WME *pattern);

    WME *retrieveFirstByFunctor(Symbol *functor);
    WME *retrieveFirstBySubsumption(WME *pattern);
    WME *retrieveFirstByRelativeSubsumption(WME *pattern);

    bool WMEwithFunctorExists(Symbol *functor);
    int maxActivationOfWMEwithFunctor(Symbol *functor);
    int maxActivationOfWMEwithFunctor(Symbol *functor, int p1);

    WME *getRandomLongTermWME();
    void preceptionContradicts(WME *wme);

    int getCycle() {return m_time;}
    
    void addUnfreezeableFunctor(Symbol *f) {m_unfreezeable_functors.push_back(f);};

    void objectRemoved(class A4Object *o);
    
    static Symbol *s_is_a_symbol;

protected:
    int m_freeze_threshold;
    int m_time;
    
    std::vector<Symbol *> m_unfreezeable_functors;

    std::list<WME *> m_short_term_memory[SYMBOL_HASH_SIZE];
    std::list<WME *> m_short_term_memory_plain;     // this list contains ALL the elements in m_short_term_memory
                                                    // it is ueful for quick iteration, without having to go through
                                                    // the whole array.

    std::list<WME *> m_long_term_memory[SYMBOL_HASH_SIZE];
    std::list<WME *> m_long_term_memory_plain;      // see comment above
};

#endif /* defined(__A4Engine__AIMemory__) */
