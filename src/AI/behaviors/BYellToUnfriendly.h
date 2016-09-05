//
//  BYellToUnfriendly.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/27/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BYellToUnfriendly__
#define __A4Engine__BYellToUnfriendly__

#include "A4Behavior.h"

class BYellToUnfriendly : public A4Behavior {
public:
    BYellToUnfriendly(char *message, int priority);
    virtual ~BYellToUnfriendly();

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    char *m_message;
    int m_angryActivation;
};

#endif /* defined(__A4Engine__BYellToUnfriendly__) */
