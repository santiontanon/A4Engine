//
//  Ontology.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__Ontology__
#define __A4Engine__Ontology__

#include <stdio.h>
#include <vector>

#include "Symbol.h"
#include "Sort.h"

class Ontology {
public:
    Ontology();
    ~Ontology();

    Sort *newSort(Symbol *name);
    Sort *newSort(Symbol *name, Sort *parent);
    Sort *newSort(Symbol *name, Sort *parent1, Sort *parent2);

    Sort *sortExistsP(const char *name);    // use this method if you are not sure the sort exists
    Sort *getSort(const char *name);        // use this one, when you are sure it does (they do the same
                                            // but this one throws an error message if the sort does not exist).
    Sort *getSort(Symbol *name);

    std::vector<Sort *> *getAllSorts();   // the list must the deleted

protected:
    std::vector<Sort *> m_sorts[SYMBOL_HASH_SIZE];
};

#endif /* defined(__A4Engine__Ontology__) */
