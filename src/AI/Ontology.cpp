//
//  Ontology.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include "debug.h"
#include <algorithm>
#include "Ontology.h"


Ontology::Ontology()
{

}


Ontology::~Ontology()
{
    for(int i = 0;i<SYMBOL_HASH_SIZE;i++) {
        for(Sort *s:m_sorts[i]) delete s;
        m_sorts[i].clear();
    }
}


Sort *Ontology::newSort(Symbol *name)
{
    Sort *s = new Sort(name);
    m_sorts[name->hash_function()].push_back(s);
    return s;
}


Sort *Ontology::newSort(Symbol *name, Sort *parent)
{
    Sort *s = new Sort(name,parent);
    m_sorts[name->hash_function()].push_back(s);
    return s;
}


Sort *Ontology::newSort(Symbol *name, Sort *parent1, Sort *parent2)
{
    Sort *s = new Sort(name,parent1,parent2);
    m_sorts[name->hash_function()].push_back(s);
    return s;
}

Sort *Ontology::sortExistsP(const char *name)
{
    for(Sort *s:m_sorts[SymbolContainer::hash_function(name)]) {
        if (s->getName()->cmp(name)) return s;
    }
    return 0;
}


Sort *Ontology::getSort(const char *name)
{
    for(Sort *s:m_sorts[SymbolContainer::hash_function(name)]) {
        if (s->getName()->cmp(name)) return s;
    }
    output_debug_message("Could not find sort '%s'\n",name);
    return 0;
}


Sort *Ontology::getSort(Symbol *name)
{
    for(Sort *s:m_sorts[name->hash_function()]) {
        if (s->getName()->cmp(name)) return s;
    }
    output_debug_message("Could not find sort '%s'\n",name->get());
    return 0;
}


std::vector<Sort *> *Ontology::getAllSorts()
{
    std::vector<Sort *> *l = new std::vector<Sort *>();
    for(int i = 0;i<SYMBOL_HASH_SIZE;i++) {
        for(Sort *s:m_sorts[i]) {
            if (std::find(l->begin(), l->end(), s) == l->end()) l->push_back(s);
        }
    }
    return l;
}

