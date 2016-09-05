//
//  Sort.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__Sort__
#define __A4Engine__Sort__

#include <vector>
#include "Symbol.h"

class Sort {
public:
    Sort(Symbol *name);
    Sort(Symbol *name, Sort *parent);
    Sort(Symbol *name, Sort *parent1, Sort *parent2);
    ~Sort();

    Symbol *getName() {return m_name;}
    std::vector<Sort *> *getParents() {return &m_parents;}
    void addParent(Sort *s);

    bool is_a(Sort *s);
    bool is_a_internal(Sort *s);
    bool is_a(char *s);
    bool subsumes(Sort *s) {
        return s->is_a(this);
    }
    
    static void precomputeIsA();
    static void clearPrecomputedIsA();

protected:
    int m_ID;
    Symbol *m_name;
    std::vector<Sort *> m_parents;

    static int s_next_ID;
    static int *precomputedIsA;
};;

#endif /* defined(__A4Engine__Sort__) */
