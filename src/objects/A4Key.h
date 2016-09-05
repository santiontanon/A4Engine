#ifndef _A4ENGINE_KEY
#define _A4ENGINE_KEY

class A4Key : public A4Item {
public:
	A4Key(Symbol *name, Sort *sort, Symbol *id, class Animation *a1);
	virtual ~A4Key();
    
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void savePropertiesToXML(XMLwriter *w, A4Game *game);

	virtual bool isKey() {return true;}

	Symbol *getID() {return m_keyID;}

protected:
	Symbol *m_keyID;
};

#endif

