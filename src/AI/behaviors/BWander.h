//
//  BWander.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/27/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BWander__
#define __A4Engine__BWander__

#include "A4Behavior.h"

class BWander : public A4Behavior {
public:
    BWander(double forwardFactor, double pStop, int stopTime, int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    double m_forwardFactor;
    double m_pStop;
    int m_stopTime;
    int m_state;
    int m_currentDirection;
};

#endif /* defined(__A4Engine__BWander__) */
