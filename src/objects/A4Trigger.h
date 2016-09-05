//
//  A4Trigger.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/15/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4Trigger__
#define __A4Engine__A4Trigger__

class A4Trigger : public A4Object {
public:
    A4Trigger(Sort *sort, int w, int h);
    virtual ~A4Trigger();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual int getPixelWidth() {return m_w;}
    virtual int getPixelHeight() {return m_h;}
    
    virtual bool isTrigger() {return true;}
    
    virtual bool cycle(A4Game *game);
    
protected:
    int m_w, m_h;
    bool m_triggerState;
};

#endif /* defined(__A4Engine__A4Trigger__) */
