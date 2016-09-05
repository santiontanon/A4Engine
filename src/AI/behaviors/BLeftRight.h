//
//  BLeftRight.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/31/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BLeftRight__
#define __A4Engine__BLeftRight__

#include "A4Behavior.h"

class BLeftRight : public A4Behavior {
public:
    BLeftRight(int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_state;
};


#endif /* defined(__A4Engine__BLeftRight__) */
