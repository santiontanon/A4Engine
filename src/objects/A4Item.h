#ifndef _A4ENGINE_ITEM
#define _A4ENGINE_ITEM

class A4Item : public A4Object {
public:
	A4Item(Symbol *name, Sort *sort);
	virtual ~A4Item();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

protected:
	// attributes:
	bool m_equipable;
	bool m_useUponTake;
	float m_attackBonus;
	float m_defenseBonus;
	float m_magicMultiplier;

};

#endif

