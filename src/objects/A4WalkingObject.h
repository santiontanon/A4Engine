//
//  A4WalkingObject.h
//  Aventura 4
//
//  Created by Santiago Ontanon on 8/24/16.
//  Copyright © 2016 Santiago Ontanon. All rights reserved.
//

#ifndef A4WalkingObject_h
#define A4WalkingObject_h

#include <list>

#include "A4Object.h"

class A4WalkingObject : public A4Object {
public:
    A4WalkingObject(Symbol *name, Sort *sort);
    virtual ~A4WalkingObject();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
        
    void setCanSwim(bool cs) {m_canSwim = cs;}
    void setWalkSpeed(int ws) {m_walk_speed = ws;}
    int getState() {return m_state;}
    
    virtual int getWalkSpeed() {return m_walk_speed;}
    
protected:
    // attributes:
    int m_walk_speed;    
    
    int m_direction, m_previous_direction;
    int m_state, m_previous_state;
    int m_state_cycle;
    
    // walking temporary counters (to make sure characters walk at the desired speed):
    int m_walking_counter;
    
    // some variables to make moving the character around intuitive:
    int m_continuous_direction_command_timers[A4_NDIRECTIONS];
    int m_continuous_direction_command_max_movement[A4_NDIRECTIONS];    // a command might specify a direction and a maximum amount of pixels to move in that direction
    bool m_direction_command_received_this_cycle[A4_NDIRECTIONS];
    
};


#endif /* A4WalkingObject_h */
