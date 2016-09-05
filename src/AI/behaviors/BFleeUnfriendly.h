//
//  BFleeUnfriendly.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/16/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BFleeUnfriendly__
#define __A4Engine__BFleeUnfriendly__

#include "A4Behavior.h"

class BFleeUnfriendly : public A4Behavior {
public:
    BFleeUnfriendly(int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_scaredActivation;
};

#endif /* defined(__A4Engine__BFleeUnfriendly__) */
