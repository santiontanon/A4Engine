//
//  BFollow.h
//  A4Engine
//
//  Created by Santiago Ontanon on 6/28/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BFollow__
#define __A4Engine__BFollow__

#include "A4Behavior.h"

class BFollow : public A4Behavior {
public:
    BFollow(class Symbol *location, int radius, int priority);
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    Symbol *m_target;
    int m_radius_sq;
    int m_inner_radius_sq;
};

#endif /* defined(__A4Engine__BFollow__) */
