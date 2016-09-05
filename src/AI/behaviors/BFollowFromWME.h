//
//  BFollowFromWME.h
//  A4Engine
//
//  Created by Santiago Ontanon on 6/28/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BFollowFromWME__
#define __A4Engine__BFollowFromWME__

#include "A4Behavior.h"

class BFollowFromWME : public A4Behavior {
public:
    BFollowFromWME(int radius, int priority);
    virtual ~BFollowFromWME();
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_radius_sq;
    int m_inner_radius_sq;
    Symbol *follow_symbol;
};

#endif /* defined(__A4Engine__BFollowFromWME__) */
