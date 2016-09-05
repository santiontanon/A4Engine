#ifndef _A4ENGINE_PLAYER_CHARACTER
#define _A4ENGINE_PLAYER_CHARACTER

class A4PlayerCharacter : public A4Character {
public:
	A4PlayerCharacter(Symbol *name, Sort *sort);
	virtual ~A4PlayerCharacter();

    virtual bool loadObjectAttribute(XMLNode *xml);    
    
	virtual bool isPlayer() {return true;}

protected:
};

#endif

