#ifndef _A4ENGINE_SPELL
#define _A4ENGINE_SPELL

class A4Spell : public A4Object {
public:
	A4Spell(Symbol *name, Sort *sort, int direction, int duration, int damag, float radius, A4Object *caster, Animation *a);
	virtual ~A4Spell();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void revisitObject(XMLNode *xml, A4Game *game);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual bool cycle(A4Game *game);
    
    virtual int getWalkSpeed() {return m_walk_speed;}

protected:
    int m_direction;
    int m_duration;
    int m_damage;
    float m_radius;
    A4Object *m_caster;

    int m_walking_counter;
    int m_walk_speed;
};

#endif

