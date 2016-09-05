#ifndef _A4ENGINE_OBJECT_FACTORY
#define _A4ENGINE_OBJECT_FACTORY

class A4ObjectFactory {
public:
	/*
		This is the parsed objects file definition xml
	*/
	A4ObjectFactory();
	~A4ObjectFactory();

	void addObjectDefinitions(XMLNode *xml, class A4Game *game);
	void addCharacterDefinitions(XMLNode *xml, class A4Game *game);

//	A4Object *createObjectFromJSString(char *JS, class A4Game *game);
//	A4Object *createObjectFromJSString(Expression *JS, class A4Game *game);
	A4Object *createObject(char *className, class A4Game *game, bool isPlayer, bool completeRedefinition);

    XMLNode *getObjectType(char *type);

protected:
    A4Object *createObjectFromXML(XMLNode *xml, class A4Game *game, bool isPlayer, bool completeRedefinition);

    std::vector<XMLNode *> objectTypes;	// types defined in the game xml files
};

#endif

