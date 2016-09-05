//
//  AIMemory.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include <algorithm>
#include "stdlib.h"
#include "AIMemory.h"


#define PERCEPTION_CONTRADICTION_CONSTANT   50


Symbol *AIMemory::s_is_a_symbol = 0;


AIMemory::AIMemory(int freeze_threshold)
{
    m_freeze_threshold = freeze_threshold;
    m_time = 0;

    if (AIMemory::s_is_a_symbol==0) AIMemory::s_is_a_symbol = new Symbol("is_a");
}


AIMemory::~AIMemory() {
    for(WME *wme:m_short_term_memory_plain) {
        delete wme;
    }
    for(WME *wme:m_long_term_memory_plain) {
        delete wme;
    }
}


void AIMemory::cycle(int steps) {
    std::vector<WME *> toFreeze;
    std::vector<WME *> toDelete;
    for(WME *wme:m_short_term_memory_plain) {
        if (wme->getActivation()>=m_freeze_threshold ||
            m_time - wme->getStartTime()>=m_freeze_threshold) {
            if (std::find(m_unfreezeable_functors.begin(), m_unfreezeable_functors.end(),wme->getFunctor())==m_unfreezeable_functors.end())
                toFreeze.push_back(wme);
        } else {
            wme->setActivation(wme->getActivation()-steps);
            if (wme->getActivation()<=0) {
                toDelete.push_back(wme);
            }
        }
    }

    for(WME *wme:toDelete) {
        int hash = wme->getFunctor()->hash_function();
        m_short_term_memory[hash].remove(wme);
        m_short_term_memory_plain.remove(wme);
        delete wme;
    }
    for(WME *wme:toFreeze) {
        int hash = wme->getFunctor()->hash_function();
        m_short_term_memory[hash].remove(wme);
        m_short_term_memory_plain.remove(wme);
        wme->setActivation(m_freeze_threshold);
        m_long_term_memory[hash].push_back(wme);
        m_long_term_memory_plain.push_back(wme);
    }

    m_time += steps;
}


WME *AIMemory::contains(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme2->equivalents(wme)) return wme2;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme2->equivalents(wme)) return wme2;
    }
    return 0;
}


WME *AIMemory::addShortTermWME(WME *wme)
{
    WME *wme2 = contains(wme);
    if (wme2==0) {
        int hash = wme->getFunctor()->hash_function();
        m_short_term_memory[hash].push_back(wme);
        m_short_term_memory_plain.push_back(wme);
        wme->setStartTime(m_time);
        return wme;
    } else {
        if (wme2->getActivation()<wme->getActivation()) wme2->setActivation(wme->getActivation());
        delete wme;
        return wme2;
    }
}


WME *AIMemory::addLongTermWME(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme2->equivalents(wme)) {
            // freeze WME:
            m_short_term_memory[hash].remove(wme2);
            m_short_term_memory_plain.remove(wme2);
            wme2->setActivation(m_freeze_threshold);
            m_long_term_memory[hash].push_back(wme2);
            m_long_term_memory_plain.push_back(wme2);
            delete wme;
            return wme2;
        }
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme2->equivalents(wme)) {
            delete wme;
            return wme2;
        }
    }

    wme->setActivation(m_freeze_threshold);
    m_long_term_memory[hash].push_back(wme);
    m_long_term_memory_plain.push_back(wme);
    wme->setStartTime(m_time);
    return wme;
}


void AIMemory::removeWME(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    m_short_term_memory[hash].remove(wme);
    m_short_term_memory_plain.remove(wme);
    m_long_term_memory[hash].remove(wme);
    m_long_term_memory_plain.remove(wme);
}


std::vector<WME *> *AIMemory::retrieveByFunctor(Symbol *functor)
{
    std::vector<WME *> *l = new std::vector<WME *>();
    int hash = functor->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(functor)) l->push_back(wme2);
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(functor)) l->push_back(wme2);
    }
    return l;
}


std::vector<WME *> *AIMemory::retrieveSubsumption(WME *wme)
{
    std::vector<WME *> *l = new std::vector<WME *>();
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme->subsumption(wme2)) l->push_back(wme2);
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme->subsumption(wme2)) l->push_back(wme2);
    }
    return l;
}


std::vector<WME *> *AIMemory::retrieveUnification(WME *wme)
{
    std::vector<WME *> *l = new std::vector<WME *>();
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        WME * u = wme2->unification(wme);
        if (u!=0) l->push_back(wme2);
        delete u;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        WME * u = wme2->unification(wme);
        if (u!=0) l->push_back(wme2);
        delete u;
    }
    return l;
}


std::vector<WME *> *AIMemory::retrieveRelativeSubsumption(WME *wme)
{
    std::vector<WME *> *l = new std::vector<WME *>();
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme->relativeSubsumption(wme2, this)) l->push_back(wme2);
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme->relativeSubsumption(wme2, this)) l->push_back(wme2);
    }
    return l;
}



WME *AIMemory::retrieveFirstByFunctor(Symbol *functor)
{
    int hash = functor->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(functor)) return wme2;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme2->getFunctor()->cmp(functor)) return wme2;
    }
    return 0;
}


WME *AIMemory::retrieveFirstBySubsumption(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme->subsumption(wme2)) return wme2;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme->subsumption(wme2)) return wme2;
    }
    return 0;
}


WME *AIMemory::retrieveFirstByRelativeSubsumption(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme->relativeSubsumption(wme2, this)) return wme2;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme->relativeSubsumption(wme2, this)) return wme2;
    }
    return 0;
}


bool AIMemory::isSubsumedByAnyWME(WME *wme)
{
    int hash = wme->getFunctor()->hash_function();
    for(WME *wme2:m_short_term_memory[hash]) {
        if (wme2->subsumption(wme)) return true;
    }
    for(WME *wme2:m_long_term_memory[hash]) {
        if (wme2->subsumption(wme)) return true;
    }
    return false;
}



bool AIMemory::WMEwithFunctorExists(Symbol *functor)
{
    int hash = functor->hash_function();
    if (!m_short_term_memory[hash].empty()) return true;
    if (!m_long_term_memory[hash].empty()) return true;
    return false;
}


int AIMemory::maxActivationOfWMEwithFunctor(Symbol *functor)
{
    int max = 0;
    int hash = functor->hash_function();
    for(WME *wme2:m_short_term_memory[hash])
        if (wme2->getFunctor()->cmp(functor) && wme2->getActivation()>max) max = wme2->getActivation();
    for(WME *wme2:m_long_term_memory[hash])
        if (wme2->getFunctor()->cmp(functor) && wme2->getActivation()>max) max = wme2->getActivation();
    return max;
}


int AIMemory::maxActivationOfWMEwithFunctor(Symbol *functor, int p1)
{
    int max = 0;
    int hash = functor->hash_function();
    for(WME *wme2:m_short_term_memory[hash])
        if (wme2->getFunctor()->cmp(functor) &&
            wme2->getParameterType(0) == WME_PARAMETER_INTEGER &&
            wme2->getParameter(0).m_integer == p1 &&
            wme2->getActivation()>max) max = wme2->getActivation();
    for(WME *wme2:m_long_term_memory[hash])
        if (wme2->getFunctor()->cmp(functor) &&
            wme2->getParameterType(0) == WME_PARAMETER_INTEGER &&
            wme2->getParameter(0).m_integer == p1 &&
            wme2->getActivation()>max) max = wme2->getActivation();
    return max;
}


WME *AIMemory::getRandomLongTermWME()
{
    int size = (int)m_long_term_memory_plain.size();
    if (size<=0) return 0;
    int idx = rand()%(int)m_long_term_memory_plain.size();
    std::list<WME *>::iterator it = m_long_term_memory_plain.begin();
    std::advance(it,idx);
    return *it;
}


void AIMemory::preceptionContradicts(WME *wme)
{
    wme->setActivation(wme->getActivation()-PERCEPTION_CONTRADICTION_CONSTANT);
    if (wme->getActivation()<=0) removeWME(wme);
}


void AIMemory::objectRemoved(A4Object *o)
{
    for(WME *w:m_short_term_memory_plain) {
        if (w->getSource()==o) w->setSource(0);
    }
    for(WME *w:m_long_term_memory_plain) {
        if (w->getSource()==o) w->setSource(0);
    }
}


