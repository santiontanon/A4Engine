#ifndef _A4ENGINE_PRESSURE_PLATE
#define _A4ENGINE_PRESSURE_PLATE

#define TRIGGER_PRESSURE_ITEM   1
#define TRIGGER_PRESSURE_HEAVY_ITEM 2
#define TRIGGER_PRESSURE_PLAYER 3

class A4PressurePlate : public A4Object {
public:
	A4PressurePlate(Sort *sort, Animation *pressed, Animation *released, int pr);
	virtual ~A4PressurePlate();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

	virtual bool cycle(A4Game *game);

protected:
	bool m_pressurePlateState;
	int m_pressureRequired;	// 0 : any item, 1: only heavy objects, characters/walls, 2: only players
};

#endif

