//
//  BCurious.h
//  A4Engine
//
//  Created by Santiago Ontanon on 4/5/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__BCurious__
#define __A4Engine__BCurious__

#include "A4Behavior.h"
#include "WME.h"

class BCurious : public A4Behavior {
public:
    BCurious(bool pickup, class Sort *sort, int activation, int priority);
    ~BCurious();
    
    virtual void toJSString(char *buffer);
    virtual void saveToXML(XMLwriter *w);
    
    virtual class A4CharacterCommand *execute(class A4AICharacter *a_character, class A4Game *a_game);
protected:
    bool m_pickup;
    Sort *m_sort;
    int m_activation;
    int m_lastMemotyTimeChecked;
    
    WME *m_pattern1, *m_pattern2, *m_pattern3;
};

#endif /* defined(__A4Engine__BCurious__) */
