//
//  BAttackUnfriendly.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/17/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BAttackUnfriendly__
#define __A4Engine__BAttackUnfriendly__

#include "A4Behavior.h"

class BAttackUnfriendly : public A4Behavior {
public:
    BAttackUnfriendly(int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_angryActivation;
};

#endif /* defined(__A4Engine__BAttackUnfriendly__) */
