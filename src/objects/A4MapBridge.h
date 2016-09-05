//
//  A4MapBridge.h
//  A4Engine
//
//  Created by Santiago Ontanon on 5/15/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4MapBridge__
#define __A4Engine__A4MapBridge__

#include "A4Object.h"

class A4MapBridge : public A4Object {
    friend class A4Map;
public:
    A4MapBridge(XMLNode *bridge_node);
    A4MapBridge(XMLNode *bridge_node, class A4Map *map);
    
    void link(A4MapBridge *b) {
        m_linkedTo = b;
        b->m_linkedTo = this;
    }
    bool findAvailableTargetLocation(class A4Object *o, int tile_dx, int tile_dy,int &x, int &y);
    
    virtual int getPixelWidth() {return m_dx;}
    virtual int getPixelHeight() {return m_dy;}
    
    int m_dx, m_dy;
    int m_appearDirection;
    bool m_appearWalking;
    A4MapBridge *m_linkedTo;
};

#endif /* defined(__A4Engine__A4MapBridge__) */
