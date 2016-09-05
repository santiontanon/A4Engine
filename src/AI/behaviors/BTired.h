//
//  BTired.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/16/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BTired__
#define __A4Engine__BTired__

#include "A4Behavior.h"

class BTired : public A4Behavior {
public:
    BTired(int timeWalking, int restTime, int priority);

    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    int m_timeWalkingUntilTired, m_restTime;
    int m_currentTimeWalking;
};

#endif /* defined(__A4Engine__BTired__) */
