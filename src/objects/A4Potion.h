#ifndef _A4ENGINE_POTION
#define _A4ENGINE_POTION

#include "A4Item.h"

class A4Potion : public A4Item {
public:
	A4Potion(Symbol *name, Sort *sort, Animation *a);

protected:
};

class A4HPPotion : public A4Potion {
public:
	A4HPPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
	int m_hp;
};

class A4MPPotion : public A4Potion {
public:
	A4MPPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
	int m_mp;
};

class A4XPPotion : public A4Potion {
public:
	A4XPPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
	int m_xp;
};

class A4StrengthPotion : public A4Potion {
public:
    A4StrengthPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
protected:
    int m_strength;
};

class A4ConstitutionPotion : public A4Potion {
public:
    A4ConstitutionPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
protected:
    int m_constitution;
};

class A4LifePotion : public A4Potion {
public:
    A4LifePotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
protected:
    int m_life;
};

class A4PowerPotion : public A4Potion {
public:
    A4PowerPotion(Symbol *name, Sort *sort, int v, Animation *a);
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
protected:
    int m_power;
};


#endif

