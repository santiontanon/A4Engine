#ifndef _A4ENGINE_EQUIPABLE_ITEM
#define _A4ENGINE_EQUIPABLE_ITEM

class A4EquipableItem : public A4Item {
public:
	A4EquipableItem(Symbol *name, Sort *sort, class Animation *a, int es, int attack, int defense, float magic, float movement, bool canChop, int gold);
	virtual ~A4EquipableItem();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

	virtual bool isEquipable() {return true;}

	int getEquipableSlot() {return m_equipable_slot;}
	int getAttack() {return m_attack;}
	int getDefense() {return m_defense;}
	float getMagicBonus() {return m_magic;}
    float getMovementBonus() {return m_movement;}
	bool getCanChop() {return m_canChop;}

protected:
	int m_equipable_slot;
	int m_attack;
	int m_defense;
	float m_magic;
    float m_movement;
	bool m_canChop;
};

#endif

