#ifndef _A4ENGINE_CONTAINER
#define _A4ENGINE_CONTAINER

class A4Container : public A4Item {
public:
	A4Container(Symbol *name, Sort *sort, class Animation *a);
	A4Container(Symbol *name, Sort *sort, A4Object *content, Animation *a);
	virtual ~A4Container();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    
    void addContent(A4Object *o) {m_content.push_back(o);}

protected:
	std::vector<A4Object *> m_content;
};

#endif

