#ifndef _A4ENGINE_SCROLL
#define _A4ENGINE_SCROLL

class A4Scroll : public A4Item {
public:
	A4Scroll(Symbol *name, Sort *sort, int spell, int gold, Animation *a);
	virtual ~A4Scroll();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
	int m_spell;
};

#endif

