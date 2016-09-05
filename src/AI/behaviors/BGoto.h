//
//  BGoto.h
//  A4Engine
//
//  Created by Santiago Ontanon on 5/30/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BGoto__
#define __A4Engine__BGoto__

#include "A4Behavior.h"

class BGoto : public A4Behavior {
public:
    BGoto(class Symbol *location, int radius, int priority);
    virtual ~BGoto();
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    Symbol *m_location;
    int m_radius_sq;
};

#endif /* defined(__A4Engine__BGoto__) */
