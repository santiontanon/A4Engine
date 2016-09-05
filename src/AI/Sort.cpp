//
//  Sort.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include "assert.h"
#include "debug.h"
#include <algorithm>
#include "Sort.h"


int Sort::s_next_ID = 1;
int *Sort::precomputedIsA = 0;

Sort::Sort(Symbol *name)
{
    m_name = name;
    m_ID = s_next_ID++;
    assert(precomputedIsA==0);
}


Sort::Sort(Symbol *name, Sort *parent)
{
    m_name = name;
    m_ID = s_next_ID++;
    m_parents.push_back(parent);
    assert(precomputedIsA==0);
}


Sort::Sort(Symbol *name, Sort *parent1, Sort *parent2)
{
    m_name = name;
    m_ID = s_next_ID++;
    m_parents.push_back(parent1);
    m_parents.push_back(parent2);
    assert(precomputedIsA==0);
}


Sort::~Sort() {
    if (m_name!=0) delete m_name;
    m_name = 0;
}


bool Sort::is_a(Sort *s)
{
    if (precomputedIsA!=0) {
        int offs = m_ID + s->m_ID*s_next_ID;
        if (precomputedIsA[offs]==0) {
            bool tmp = is_a_internal(s);
            precomputedIsA[offs] = (tmp ? 1:2);
            return tmp;
        } else {
            return (precomputedIsA[offs]==1 ? true:false);
        }
    } else {
        return is_a_internal(s);
    }
}


bool Sort::is_a_internal(Sort *s)
{
    if (s == this) return true;
    for(Sort *parent:m_parents) if (parent->is_a_internal(s)) return true;
    return false;
}


bool Sort::is_a(char *s)
{
    if (m_name->cmp(s)) return true;
    for(Sort *parent:m_parents) if (parent->is_a(s)) return true;
    return false;
}


void Sort::addParent(Sort *s) {
    if (std::find(m_parents.begin(), m_parents.end(), s)==m_parents.end()) {
        m_parents.push_back(s);
    }
}


void Sort::precomputeIsA() {
    precomputedIsA = new int[s_next_ID*s_next_ID];
    for(int i = 0;i<s_next_ID*s_next_ID;i++) {
        precomputedIsA[i] = 0;
    }
}

void Sort::clearPrecomputedIsA() {
    delete []precomputedIsA;
    precomputedIsA = 0;
}

