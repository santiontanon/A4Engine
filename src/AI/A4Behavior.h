//
//  A4Behavior.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/31/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4Behavior__
#define __A4Engine__A4Behavior__

#include "XMLwriter.h"

class A4Behavior {
public:
    A4Behavior(int priority);
    A4Behavior(int priority, char *ID);
    virtual ~A4Behavior();
    
    static A4Behavior *createBehaviorFromXML(class XMLNode *xml, class Ontology *o);
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w) = 0;

    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);

    void setPriority(int p) {m_priority = p;}
    int getPriority() {return m_priority;}
    
    void setID(char *ID) {m_ID = ID;}
    char *getID() {return m_ID;}

protected:
    int m_priority;
    char *m_ID;
};

#endif /* defined(__A4Engine__A4Behavior__) */
