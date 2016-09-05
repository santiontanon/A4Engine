//
//  A4Agenda.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 7/4/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include "stdlib.h"
#include "A4Agenda.h"
#include "A4Script.h"
#include "XMLparser.h"
#include "A4Object.h"
#include "A4Character.h"
#include "A4Map.h"
#include "A4Game.h"


AgendaEntry::AgendaEntry(XMLNode *xml)
{
    time = atoi(xml->get_attribute("time"));
    for(XMLNode *script_xml:*(xml->get_children())) {
        m_scripts.push_back(new A4Script(script_xml));
    } 
}


AgendaEntry::AgendaEntry(AgendaEntry *ae)
{
    time = ae->time;
    for(A4Script *s:ae->m_scripts) {
        m_scripts.push_back(new A4Script(s));
    }
}


AgendaEntry::~AgendaEntry()
{
    for(A4Script *s:m_scripts) delete s;
    m_scripts.clear();
}


int AgendaEntry::execute(A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    int retValue = SCRIPT_FINISHED;
    A4ScriptExecutionQueue *seq = 0;
    for(A4Script *s:m_scripts) {
        if (seq==0) {
            s->reset();
            retValue = s->execute(o, map, game, otherCharacter);
            if (retValue == SCRIPT_FINISHED) {
                // good, do nothing
            } else if (retValue == SCRIPT_NOT_FINISHED) {
                // script needs more time, create an script queue
                seq = new A4ScriptExecutionQueue(o, map, game, otherCharacter);
                seq->scripts.push_back(new A4Script(s));
                if (o!=0 ) {
                    o->addScriptQueue(seq);
                } else if (map!=0) {
                    map->addScriptQueue(seq);
                } else {
                    game->addScriptQueue(seq);
                }
            } else {
                // failed, stop the script
                break;
            }
        } else {
            s->reset();
            seq->scripts.push_back(new A4Script(s));
        }
    }
    return retValue;
}


Agenda::Agenda(XMLNode *xml)
{
    char *tmp = xml->get_attribute("agenda");
    name = new char[strlen(tmp)+1];
    strcpy(name,tmp);
    duration = atoi(xml->get_attribute("duration"));
    cycle = 0;
    if (xml->get_attribute("cycle")!=0)
        cycle = atoi(xml->get_attribute("cycle"));

    loop = false;
    char *loop_text = xml->get_attribute("loop");
    if (loop_text!=0 && strcmp(loop_text,"true")==0) loop = true;

    absoluteTime = false;
    char *absolute_text = xml->get_attribute("absoluteTime");
    if (absolute_text!=0 && strcmp(absolute_text,"true")==0) absoluteTime = true;
    
    for(XMLNode *entry_xml:*(xml->get_children())) {
        m_entries.push_back(new AgendaEntry(entry_xml));
    }
    absoluteStartCycle = -1;
}


Agenda::Agenda(Agenda *a)
{
    name = new char[strlen(a->name)+1];
    strcpy(name,a->name);
    duration = a->duration;
    loop = a->loop;
    absoluteTime = a->absoluteTime;
    cycle = 0;
    for(AgendaEntry *e:a->m_entries) {
        m_entries.push_back(new AgendaEntry(e));
    }
    absoluteStartCycle = -1;
}


Agenda::~Agenda()
{
    delete name;
    name = 0;
    for(AgendaEntry *ae:m_entries) delete ae;
    m_entries.clear();
}


bool Agenda::execute(A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (absoluteTime) {
        if (absoluteStartCycle<0) absoluteStartCycle = game->getCycle();
        cycle = game->getCycle() - absoluteStartCycle;
    }
    for(AgendaEntry *ae:m_entries) {
        if (ae->time == (cycle%duration)) {
            // execute entry!
            ae->execute(o,map,game,otherCharacter);
        }
    }
    if (!absoluteTime) cycle++;
    if (cycle>=duration && !loop) {
        if (!loop) return true;
    }
    return false;
}


