//
//  A4Agenda.h
//  A4Engine
//
//  Created by Santiago Ontanon on 7/4/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4Agenda__
#define __A4Engine__A4Agenda__

#include <vector>


class AgendaEntry {
public:
    AgendaEntry(class XMLNode *xml);
    AgendaEntry(AgendaEntry *ae);
    ~AgendaEntry();
    
    int execute(class A4Object *o, class A4Map *map, class A4Game *game, class A4Character *otherCharacter);

    int time;
    std::vector<class A4Script *> m_scripts;
};


class Agenda {
public:
    Agenda(XMLNode *xml);
    Agenda(Agenda *a);
    ~Agenda();
    
    bool execute(class A4Object *o, class A4Map *map, class A4Game *game, class A4Character *otherCharacter);
    
    char *name;
    int duration;
    bool loop,absoluteTime;
    int absoluteStartCycle;
    int cycle;
    std::vector<AgendaEntry *> m_entries;
};


#endif /* defined(__A4Engine__A4Agenda__) */
