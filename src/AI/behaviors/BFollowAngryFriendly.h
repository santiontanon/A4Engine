//
//  BFollowAngryFriendly.h
//  A4Engine
//
//  Created by Santiago Ontanon on 7/4/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BFollowAngryFriendly__
#define __A4Engine__BFollowAngryFriendly__

#include "A4Behavior.h"

class BFollowAngryFriendly : public A4Behavior {
public:
    BFollowAngryFriendly(int radius, int priority);
    virtual ~BFollowAngryFriendly();
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_radius_sq;
    int m_inner_radius_sq;
    WME *pattern;
};

#endif /* defined(__A4Engine__BFollowAngryFriendly__) */
