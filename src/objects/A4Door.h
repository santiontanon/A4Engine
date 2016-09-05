#ifndef _A4ENGINE_DOOR
#define _A4ENGINE_DOOR

class A4Door : public A4Object {
public:
	A4Door(Sort *sort, Symbol *ID, bool closed, bool consumeKey, class Animation *a_closed, Animation *a_open);
	virtual ~A4Door();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    Symbol *getDoorGroupID() {
        return m_doorGroupID;
    }
    
	virtual bool isWalkable() {return !m_closed;}
	virtual bool isInteracteable() {return true;}

	virtual void event(int event, class A4Character *character, class A4Map *map, class A4Game *game);
	virtual void eventWithID(int event, char *ID, class A4Character *character, class A4Map *map, class A4Game *game);

    virtual bool isDoor() {return true;}
    
    void changeStateRecursively(bool closed, A4Character *character, A4Map *map, A4Game *game);
    bool checkForBlockages(bool closed, A4Character *character, A4Map *map, A4Game *game);
    bool checkForBlockages(bool closed, A4Character *character, A4Map *map, A4Game *game, std::list<A4Door *> *alreadyVisited);

    virtual int getPixelWidth();
    virtual int getPixelHeight();
    
protected:
	Symbol *m_doorID;
    Symbol *m_doorGroupID;
	bool m_closed;
    bool m_consumeKey;
};

#endif

