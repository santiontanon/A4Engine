#ifndef _A4ENGINE_VEHICLE
#define _A4ENGINE_VEHICLE

#include <list>

#include "A4WalkingObject.h"

class A4Vehicle : public A4WalkingObject {
public:
	A4Vehicle(Symbol *name, Sort *sort);
	virtual ~A4Vehicle();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void revisitObject(XMLNode *xml, A4Game *game);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual bool cycle(A4Game *game);

    virtual void issueCommand(int command, int argument, int direction, A4Object *target, class A4Game *game);

    void embark(A4Object *l) {m_load.push_back(l);}
    void disembark(A4Object *l) {m_load.remove(l);};
    bool takeDamage(int damage);
    bool isEmpty() {return m_load.empty();}

    int getHp() {return m_hp;}
    void setHp(int hp) {m_hp = hp;}
    void setMaxHp(int hp) {m_max_hp = hp;}
    void setCanSwim(bool cs) {m_canSwim = cs;}
    void setMagicImmune(bool m) {m_magicImmune = m;}
    int getState() {return m_state;}
    
    virtual bool isWalkable() {return false;}   // vehicles are not walkable, there is a special case in the collision function the maps
                                                // that makes them walkable to characters
    virtual bool isHeavy() {return true;}		// this is used by pressure plates
    virtual bool isVehicle() {return true;}

protected:
    // attributes:
    int m_hp, m_max_hp;
    bool m_magicImmune;

    std::list<A4Object *> m_load;
};

#endif

