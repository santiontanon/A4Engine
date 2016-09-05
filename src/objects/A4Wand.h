//
//  A4Wand.hpp
//  Aventura 4
//
//  Created by Santiago Ontanon on 8/28/16.
//  Copyright © 2016 Santiago Ontanon. All rights reserved.
//

#ifndef A4Wand_h
#define A4Wand_h

class A4Wand : public A4Item {
public:
    A4Wand(Symbol *name, Sort *sort, int spell, int charges, bool disappear, int gold, Animation *a);
    virtual ~A4Wand();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual void eventWithInteger(int event, int value, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
    virtual bool isWand() {return true;}
    
protected:
    int m_spell;
    int m_charges;
    bool m_disappearWhenEmpty;
};


#endif /* A4Wand_hpp */
