//
//  BWanderInCircles.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/28/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BWanderInCircles__
#define __A4Engine__BWanderInCircles__

#include "A4Behavior.h"

class BWanderInCircles : public A4Behavior {
public:
    BWanderInCircles(int diameter, int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_diameter;
    int m_state;
    int m_currentDirection;
};

#endif /* defined(__A4Engine__BWanderInCircles__) */
