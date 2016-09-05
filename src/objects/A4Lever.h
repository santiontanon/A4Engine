#ifndef _A4ENGINE_LEVER
#define _A4ENGINE_LEVER

class A4Lever : public A4Object {
public:
	A4Lever(Sort *sort, Symbol *ID, bool state, class Animation *a_closed, Animation *a_open);
	virtual ~A4Lever();

    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    
	virtual void event(int event, class A4Character *character, class A4Map *map, class A4Game *game);

protected:
	Symbol *m_leverID;
	bool m_leverState;
};

#endif

